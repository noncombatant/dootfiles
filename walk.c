// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <dirent.h>
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

// clang-format off
static unsigned char help[] =
"-0      delimit records with NUL instead of newline\n"
"-A      walk hidden files too\n"
"-a date-time\n"
"        match files modified after the given date-time\n"
"-b date-time\n"
"        matchfiles modified before the given date-time\n"
"-d depth\n"
"        descend at most this many directory levels below the argument(s)\n"
"-h      print this help message\n"
"-m pattern\n"
"        match files whose pathnames match the regular expression\n"
"-S size\n"
"        match files larger than the given size\n"
"-s size\n"
"        match files smaller than the given size\n"
"-t types\n"
"        match files of the given file type(s)"
"\n"
"Regular expressions are case-insensitive POSIX Extended expressions; refer to re_format(7).\n"
"\n"
"Date-times are in the format %Y-%m-%d %H:%M:%S, %Y-%m-%d, or %H:%M:%S; refer to strptime(3).\n"
"\n"
"File types is a string containing 1 or more of 'd'irectory, 'f'file, or 's'ymbolic link characters.\n"
"\n"
"Sizes can be given in any base; refer to strtoll(3).\n";
// clang-format on

static char ORS = '\n';

static void noreturn PrintHelp(int error) {
  fprintf(error == 0 ? stdout : stderr, "%s", help);
  exit(error);
}

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
  long long depth;
  bool has_larger_than;
  long long larger;
  bool has_smaller_than;
  long long smaller;
  bool has_type;
  Type type;
} Predicate;

typedef enum Result {
  ResultContinue = 0,
  ResultNoDescend = 1,
} Result;

static Result PrintIfMatch(const char* pathname,
                           const struct dirent* entry,
                           const Predicate* p) {
  if (entry->d_name[0] == '.' && !p->walk_all) {
    return ResultNoDescend;
  }

  // These tests are arranged such that the least expensive are run first.
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
      p->has_smaller_than) {
    struct stat status;
    if (lstat(pathname, &status)) {
      perror(pathname);
      return ResultContinue;
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

  printf("%s%c", pathname, ORS);
  return ResultContinue;
}

static time_t ParseDateTime(const char* string) {
  struct tm date_time = {0};
  if (strptime(string, "%Y-%m-%d %H:%M:%S", &date_time)) {
    return mktime(&date_time);
  }

  if (strptime(string, "%Y-%m-%d", &date_time)) {
    return mktime(&date_time);
  }

  const time_t t = time(NULL);
  date_time = *localtime(&t);
  if (strptime(string, "%H:%M:%S", &date_time)) {
    return mktime(&date_time);
  }
  PrintHelp(1);
}

static void ParseRE(const char* string, regex_t* pattern) {
  const int e = regcomp(pattern, string, REG_EXTENDED | REG_ICASE | REG_NOSUB);
  if (e != 0) {
    char message[512] = "";
    (void)regerror(e, pattern, message, sizeof(message));
    fprintf(stderr, "could not compile RE: %s\n", message);
    PrintHelp(1);
  }
}

static long long ParseInt(const char* string) {
  char* end;
  long long r = strtoll(string, &end, 0);
  if (*end != '\0') {
    PrintHelp(1);
  }
  return r;
}

static void Walk(const char* root, long long depth, const Predicate* p) {
  if (depth > p->depth) {
    return;
  }
  DIR* d = opendir(root);
  if (d == NULL) {
    perror(root);
    return;
  }

  while (true) {
    const struct dirent* entry = readdir(d);
    if (entry == NULL) {
      break;
    }
    if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) {
      continue;
    }

    char pathname[PATH_MAX + 1] = "";
    const int length =
        snprintf(pathname, sizeof(pathname), "%s/%s", root, entry->d_name);
    if (length < 0 || (size_t)length >= sizeof(pathname)) {
      fprintf(stderr, "can't handle %s/%s\n", root, entry->d_name);
      continue;
    }

    const Result r = PrintIfMatch(pathname, entry, p);
    if (r != ResultNoDescend && entry->d_type & DT_DIR) {
      Walk(pathname, depth + 1, p);
    }
  }

  if (closedir(d)) {
    perror(root);
  }
}

int main(int count, char* arguments[]) {
  Predicate p = {0};
  opterr = 0;
  while (true) {
    const int ch = getopt(count, arguments, "0Aa:b:d:hm:S:s:t:");
    if (ch == -1) {
      break;
    }
    switch (ch) {
      case '0':
        ORS = '\0';
        break;
      case 'A':
        p.walk_all = true;
        break;
      case 'a':
        p.after = ParseDateTime(optarg);
        p.has_after = true;
        break;
      case 'b':
        p.before = ParseDateTime(optarg);
        p.has_before = true;
        break;
      case 'd':
        p.depth = ParseInt(optarg);
        break;
      case 'h':
        PrintHelp(0);
      case 'm': {
        ParseRE(optarg, &p.pattern);
        p.has_pattern = true;
        break;
      }
      case 'S':
        p.larger = ParseInt(optarg);
        p.has_larger_than = true;
        break;
      case 's':
        p.smaller = ParseInt(optarg);
        p.has_smaller_than = true;
        break;
      case 't':
        p.has_type = true;
        if (strchr(optarg, 'f')) {
          p.type |= TypeFile;
        }
        if (strchr(optarg, 'd')) {
          p.type |= TypeDirectory;
        }
        if (strchr(optarg, 's')) {
          p.type |= TypeSymbolicLink;
        }
        break;
      default:
        PrintHelp(1);
    }
  }
  count -= optind;
  arguments += optind;

  if (count == 0) {
    Walk(".", 0, &p);
  }
  for (int i = 0; i < count; i++) {
    Walk(arguments[i], 0, &p);
  }
}