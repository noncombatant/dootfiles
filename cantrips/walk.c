// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] =
"walk directory trees and print pathnames matching search terms\n"
"\n"
"    walk [options...] [pathnames...]\n"
"\n"
"Patterns are case-insensitive POSIX extended regular expressions; refer to re_format(7).\n"
"\n"
"Date-times are in the format %Y-%m-%d %H:%M:%S, %Y-%m-%d, or %H:%M:%S; refer to strptime(3).\n"
"\n"
"File types is a string containing 1 or more of 'd'irectory, 'f'file, or 's'ymbolic link characters.\n"
"\n"
"Sizes can be given in any base; refer to strtoll(3).";

static Option options[] = {
  {
    .flag = '0',
    .description = "delimit output records with NUL instead of newline",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'A',
    .description = "walk hidden files too",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'a',
    .description = "match files modified after",
    .value = { .type = OptionTypeDateTime }
  },
  {
    .flag = 'b',
    .description = "match files modified before",
    .value = { .type = OptionTypeDateTime }
  },
  {
    .flag = 'd',
    .description = "descend at most this many directory levels below the argument(s)",
    .value = { .type = OptionTypeInt }
  },
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'm',
    .description = "match files whose pathnames match",
    .value = { .type = OptionTypeRegex }
  },
  {
    .flag = 'S',
    .description = "match files larger than this size",
    .value = { .type = OptionTypeInt }
  },
  {
    .flag = 's',
    .description = "match files smaller than this size",
    .value = { .type = OptionTypeInt }
  },
  {
    .flag = 't',
    .description = "match files of the given file types",
    .value = { .type = OptionTypeString }
  },
  {
    .flag = 'u',
    .description = "search up the directory hierarchy rather than down",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'x',
    .description = "do not cross a device boundary when walking",
    .value = { .type = OptionTypeBool }
  },
};

static CLI cli = {
  .name = "walk",
  .description = description,
  .options = {.count = COUNT(options), .values = options},
};
// clang-format on

#define OVB(flag) FindOptionValue(cli.options, (flag))->b
#define OVDT(flag) FindOptionValue(cli.options, (flag))->dt
#define OVRE(flag) FindOptionValue(cli.options, (flag))->r
#define OVS(flag) FindOptionValue(cli.options, (flag))->s
#define OVZ(flag) FindOptionValue(cli.options, (flag))->z
#define OVI(flag) FindOptionValue(cli.options, (flag))->i

static char ors = '\n';

typedef enum Type {
  TypeNone = 0,
  TypeFile = 1,
  TypeDirectory = 2,
  TypeSymbolicLink = 4,
} Type;

typedef struct Predicate {
  bool walk_all;
  bool has_pattern;
  regex_t pattern;
  bool has_after;
  time_t after;
  bool has_before;
  time_t before;
  bool has_depth;
  long long depth;
  bool has_larger_than;
  long long larger;
  bool has_smaller_than;
  long long smaller;
  bool has_type;
  Type type;
  bool has_no_cross_device;
  dev_t device;
} Predicate;

typedef enum Result {
  ResultContinue = 0,
  ResultStop = 1,
} Result;

static Result PrintIfMatch(const char* pathname,
                           const struct dirent* entry,
                           const Predicate* p) {
  if (entry->d_name[0] == '.' && !p->walk_all) {
    return ResultStop;
  }

  // These tests are arranged such that the least expensive run first.
  if (p->has_type) {
    Type expected = TypeNone;
    if (entry->d_type == DT_REG) {
      expected = TypeFile;
    } else if (entry->d_type == DT_DIR) {
      expected = TypeDirectory;
    } else if (entry->d_type == DT_LNK) {
      expected = TypeSymbolicLink;
    }
    if ((p->type & expected) == 0) {
      return ResultContinue;
    }
  }

  if (p->has_pattern && regexec(&p->pattern, pathname, 0, NULL, 0) != 0) {
    return ResultContinue;
  }

  if (p->has_after || p->has_before || p->has_larger_than ||
      p->has_smaller_than || p->has_no_cross_device) {
    struct stat status;
    if (lstat(pathname, &status)) {
      Warn(errno, "%s", pathname);
      return ResultContinue;
    }

    if (p->has_no_cross_device && p->device != status.st_dev) {
      return ResultStop;
    }

    if ((p->has_after && status.st_mtime <= p->after) ||
        (p->has_before && status.st_mtime >= p->before)) {
      return ResultContinue;
    }

    if ((p->has_larger_than && status.st_size <= p->larger) ||
        (p->has_smaller_than && status.st_size >= p->smaller)) {
      return ResultContinue;
    }
  }

  MustPrintf(stdout, "%s%c", pathname, ors);
  return ResultContinue;
}

static bool IsDotOrDotDot(const char* basename) {
  return StringEquals(".", basename) || StringEquals("..", basename);
}

static void Walk(const char* root, const Predicate* p, long depth) {
  if (p->has_depth && depth > p->depth) {
    return;
  }
  AUTO(DIR*, d, opendir(root), CloseDir);
  if (!d) {
    Warn(errno, "%s", root);
    return;
  }

  while (true) {
    const struct dirent* entry = readdir(d);
    if (!entry) {
      break;
    }
    if (IsDotOrDotDot(entry->d_name)) {
      continue;
    }

    char pathname[PATH_MAX + 1] = "";
    const size_t length =
        Format(pathname, sizeof(pathname), "%s/%s", root, entry->d_name);
    if (length == SIZE_MAX) {
      MustPrintf(stderr, "can't handle %s/%s\n", root, entry->d_name);
      continue;
    }

    const Result r = PrintIfMatch(pathname, entry, p);
    if (r != ResultStop && entry->d_type & DT_DIR) {
      Walk(pathname, p, depth + 1);
    }
  }
}

static void WalkUp(char* pathname, const Predicate* p, long long depth) {
  if (p->has_depth && depth > p->depth) {
    return;
  }
  AUTO(DIR*, d, opendir(pathname), CloseDir);
  if (!d) {
    Warn(errno, "%s", pathname);
    return;
  }

  while (true) {
    const struct dirent* entry = readdir(d);
    if (!entry) {
      break;
    }
    if (IsDotOrDotDot(entry->d_name)) {
      continue;
    }

    char child[PATH_MAX + 1] = "";
    const size_t length =
        Format(child, sizeof(child), "%s/%s", pathname, entry->d_name);
    if (length == SIZE_MAX) {
      MustPrintf(stderr, "can't handle %s/%s\n", pathname, entry->d_name);
      continue;
    }
    // TODO: Check return value.
    PrintIfMatch(child, entry, p);
  }

  if (!StringEquals("/", pathname)) {
    WalkUp(dirname(pathname), p, depth + 1);
  }
}

static int PopulateDevice(Predicate* p, const char* pathname) {
  if (!p->has_no_cross_device) {
    return 0;
  }
  struct stat status;
  if (lstat(pathname, &status)) {
    return errno;
  }
  p->device = status.st_dev;
  return 0;
}

int main(int count, char** arguments) {
  Arguments as = ParseCLI(&cli, count, arguments);
  if (OVB('h')) {
    PrintHelpAndExit(&cli, false, true);
  }

  // Since finding options and values in `cli` takes linear time, and since we'd
  // be looking them up for every pathname we try to match, we instead copy the
  // options into the `Predicate` `struct` for constant-time lookup.
  Predicate p = {0};
  ors = OVB('0') ? '\0' : '\n';
  p.walk_all = OVB('A');
  bool up = OVB('u');
  if (OVB('a')) {
    p.after = OVDT('a');
    p.has_after = true;
  }
  if (OVB('b')) {
    p.before = OVDT('b');
    p.has_before = true;
  }
  if (OVB('d')) {
    p.depth = OVI('d');
    p.has_depth = true;
  }
  if (OVB('m')) {
    p.pattern = OVRE('m');
    p.has_pattern = true;
  }
  if (OVB('S')) {
    p.larger = OVI('S');
    p.has_larger_than = true;
  }
  if (OVB('s')) {
    p.smaller = OVI('s');
    p.has_smaller_than = true;
  }
  if (OVB('t')) {
    const char* s = OVS('t');
    if (strchr(s, 'f')) {
      p.type |= TypeFile;
    }
    if (strchr(s, 'd')) {
      p.type |= TypeDirectory;
    }
    if (strchr(s, 's')) {
      p.type |= TypeSymbolicLink;
    }
    p.has_type = true;
  }
  p.has_no_cross_device = OVB('x');

  if (as.count == 0) {
    if (up) {
      char cwd[PATH_MAX + 1] = "";
      getcwd(cwd, sizeof(cwd));
      const int e = PopulateDevice(&p, cwd);
      if (e) {
        MustPrintf(stderr, "%s: %s\n", cwd, strerror(e));
        return errno;
      }
      WalkUp(cwd, &p, 0);
    } else {
      const int e = PopulateDevice(&p, ".");
      if (e) {
        MustPrintf(stderr, "./: %s\n", strerror(e));
        return errno;
      }
      Walk(".", &p, 0);
    }
  }
  for (size_t i = 0; i < as.count; i++) {
    const int e = PopulateDevice(&p, as.values[i]);
    if (e) {
      MustPrintf(stderr, "./: %s\n", strerror(e));
      continue;
    }
    if (up) {
      WalkUp(as.values[i], &p, 0);
    } else {
      Walk(as.values[i], &p, 0);
    }
  }
}
