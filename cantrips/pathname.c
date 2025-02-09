// Copyright 2024 Chris Palmer, https://noncombatant.org/, with a substantial
// portion by Russ Cox copyright 2003.
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] =
"print properties of pathnames\n"
"\n"
"    pathname [options...] [pathnames...]\n"
"\n"
"Note that because it canonicalizes the strings first, this program may produce different results than basename(1) and dirname(1).";

static Option options[] = {
  {
    .flag = 'b',
    .description = "canonicalize the given pathnames and print their basenames",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'c',
    .description = "canonicalize the given pathnames and print them",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'd',
    .description = "canonicalize the given pathnames and print their dirnames",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'e',
    .description = "canonicalize the given pathnames and print their file extensions, if any",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool }
  },
};

static CLI cli = {
  .name = "pathname",
  .description = description,
  .options = {.count = COUNT(options), .values = options},
};
// clang-format on

static const char path_separator = '/';

static bool is_separator(char c) {
  return c == path_separator || c == '\0';
}

// Rewrites `string` in place to compress consecutive '/'s, eliminate '.'
// pathname components, and process "..". Returns an alias into `string`.
//
// Adapted from
// https://github.com/9fans/plan9port/blob/61e362add9e1485bec1ab8261d729016850ec270/src/lib9/cleanname.c
// by Russ Cox, released under the MIT license.
static char* LexicallyCanonicalizePathname(char* string) {
  if (string[0] == '\0') {
    return string;
  }

  char *p, *q, *dotdot;
  bool rooted = string[0] == path_separator;

  // Invariants:
  //
  // * `p` points at beginning of the path element we're considering.
  // * `q` points just past the last path element we wrote (no slash).
  // * `dotdot` points just past the point where .. cannot backtrack
  //   any further (no slash).
  p = q = dotdot = string + rooted;
  while (*p) {
    if (p[0] == path_separator) {  // null element
      p++;
    } else if (p[0] == '.' && is_separator(p[1])) {
      p += 1;  // don't count the separator in case it is NUL
    } else if (p[0] == '.' && p[1] == '.' && is_separator(p[2])) {
      p += 2;
      if (q > dotdot) {  // can backtrack
        while (--q > dotdot && *q != path_separator) {
        }
      } else if (!rooted) {  // /.. is / but ./../ is ..
        if (q != string) {
          *q++ = path_separator;
        }
        *q++ = '.';
        *q++ = '.';
        dotdot = q;
      }
    } else {  // real path element
      if (q != string + rooted) {
        *q++ = path_separator;
      }
      while ((*q = *p) != path_separator && *q != 0) {
        p++;
        q++;
      }
    }
  }

  if (q == string) {  // empty string is really "."
    *q++ = '.';
  }
  *q = '\0';
  return string;
}

typedef struct Test {
  char* pathname;
  char* want;
} Test;

static Bytes PathnameFromString(char* string) {
  char* canonical = LexicallyCanonicalizePathname(string);
  return (Bytes){.count = strlen(canonical), .values = canonical};
}

// Returns the index in `b.values` where the basename begins.
static size_t Basename(Bytes b) {
  const size_t i = LastIndex(b.values, b.count, path_separator);
  if (i == not_found) {
    return 0;
  }
  if (i == 0 && b.values[i] == '/') {
    return 0;
  }
  return i + 1;
}

// Returns the length of `b.values`’s dirname (which will be 0, if `b.values` is
// a basename).
static size_t Dirname(Bytes b) {
  const size_t i = LastIndex(b.values, b.count, path_separator);
  if (i == not_found) {
    return 0;
  }
  if (i == 0 && b.values[i] == '/') {
    return 1;
  }
  return i;
}

// Returns the index in the basename of `b.values` of the last '.', or
// `not_found`.
static size_t Extension(Bytes b) {
  const size_t basename = Basename(b);
  const size_t dot = LastIndex(&b.values[basename], b.count - basename, '.');
  if (dot == not_found || (basename == 0 && dot == 0)) {
    return not_found;
  }
  return basename + dot;
}

#if defined(TEST)
static void TestLexicallyCanonicalizePathname() {
  Test tests[] = {
      {"/goat/bloat/../../../../../etc/passwd", "/etc/passwd"},
      {"goat/blorp/././///yow", "goat/blorp/yow"},
      {"/goat/blorp/../../../../../", "/"},
      {"", ""},
      {"/leg/foot///////////", "/leg/foot"},
      {".", "."},
      {"/", "/"},
      {"./", "."},
      {"../goat/../", ".."},
  };
  for (size_t i = 0; i < COUNT(tests); i++) {
    AUTO(char*, copy, strdup(tests[i].pathname), FreeChar);
    const char* canonicalized = LexicallyCanonicalizePathname(copy);
    if (!StringEquals(canonicalized, tests[i].want)) {
      MustPrintf(stderr, "Lexically '%s': wanted '%s', got '%s'\n",
                 tests[i].pathname, tests[i].want, canonicalized);
    }
  }
}

static void TestBasename() {
  Test tests[] = {
      {"/goat/bloat/../../../../../etc/passwd", "passwd"},
      {"goat/blorp/././///yow", "yow"},
      {"/goat/blorp/../../../../../", "/"},
      {"", ""},
      {"/leg/foot///////////", "foot"},
      {".", "."},
      {"/", "/"},
      {"./", "."},
      {"../goat/../", ".."},
      {"/etc/passwd", "passwd"},
  };
  for (size_t i = 0; i < COUNT(tests); i++) {
    AUTO(char*, copy, strdup(tests[i].pathname), FreeChar);
    const Bytes bytes = PathnameFromString(copy);
    const size_t b = Basename(bytes);
    const char* basename = &bytes.values[b];
    if (!StringEquals(basename, tests[i].want)) {
      MustPrintf(stderr,
                 "Basename '%s' (canonical: '%s'): wanted '%s', got '%s'\n",
                 tests[i].pathname, bytes.values, tests[i].want, basename);
    }
  }
}

static void TestDirname() {
  Test tests[] = {
      {"/goat/bloat/../../../../../etc/passwd", "/etc"},
      {"goat/blorp/././///yow", "goat/blorp"},
      {"/goat/blorp/../../../../../", "/"},
      {"", "."},
      {"/leg/foot///////////", "/leg"},
      {".", "."},
      {"/", "/"},
      {"./", "."},
      {"../goat/../", "."},
  };
  for (size_t i = 0; i < COUNT(tests); i++) {
    AUTO(char*, copy, strdup(tests[i].pathname), FreeChar);
    const Bytes bytes = PathnameFromString(copy);
    const size_t d = Dirname(bytes);
    if (d) {
      bytes.values[d] = '\0';
    }
    const char* basename = d ? bytes.values : ".";
    if (!StringEquals(basename, tests[i].want)) {
      MustPrintf(stderr, "Dirname '%s': wanted '%s', got '%s'\n",
                 tests[i].pathname, tests[i].want, basename);
    }
  }
}

static void TestExtension() {
  Test tests[] = {
      {"/goat/bloat/../../../../../etc/passwd", ""},
      {"goat/blorp/././///yow.txt", ".txt"},
      {"/goat/blorp/../../../../../pdf.exe.c", ".c"},
      {"", ""},
      {"/leg/foot///////////stuff.rs", ".rs"},
      {".", ""},
      {"/", ""},
      {"./", ""},
      {"../goat/../things.db", ".db"},
  };
  for (size_t i = 0; i < COUNT(tests); i++) {
    AUTO(char*, copy, strdup(tests[i].pathname), FreeChar);
    const Bytes bytes = PathnameFromString(copy);
    const size_t e = Extension(bytes);
    const char* extension = e == not_found ? "" : &bytes.values[e];
    if (!StringEquals(extension, tests[i].want)) {
      MustPrintf(stderr,
                 "Extension '%s' (canonical: '%s'): wanted '%s', got '%s'\n",
                 tests[i].pathname, bytes.values, tests[i].want, extension);
    }
  }
}
#endif

static void PrintWithLabel(const char* label, const char* string) {
  if (label) {
    MustPrintf(stdout, "%-12s%s\n", label, string);
  } else {
    MustPrintf(stdout, "%s\n", string);
  }
}

int main(int count, char** arguments) {
#if defined(TEST)
  TestLexicallyCanonicalizePathname();
  TestBasename();
  TestDirname();
  TestExtension();
#endif

  Arguments as = ParseCLI(&cli, count, arguments);
  if (FindOptionValue(cli.options, 'h')->b) {
    PrintHelpAndExit(&cli, false, true);
  }
  if (as.count == 0) {
    PrintHelpAndExit(&cli, true, true);
  }

  const bool print_basename = FindOptionValue(cli.options, 'b')->b;
  const bool print_canonical = FindOptionValue(cli.options, 'c')->b;
  const bool print_dirname = FindOptionValue(cli.options, 'd')->b;
  const bool print_extension = FindOptionValue(cli.options, 'e')->b;
  const int total =
      (int)print_basename + print_canonical + print_dirname + print_extension;
  if (total == 0) {
    PrintHelpAndExit(&cli, true, true);
  }
  const bool multiple = total > 1;

  for (size_t i = 0; i < as.count; i++) {
    AUTO(char*, copy, strdup(as.values[i]), FreeChar);
    const Bytes p = PathnameFromString(copy);
    if (print_canonical) {
      PrintWithLabel(multiple ? "canonical" : NULL, p.values);
    }
    if (print_dirname) {
      const size_t d = Dirname(p);
      if (d) {
        const char previous = p.values[d];
        p.values[d] = '\0';
        PrintWithLabel(multiple ? "dirname" : NULL, p.values);
        p.values[d] = previous;
      } else {
        PrintWithLabel(multiple ? "dirname" : NULL, ".");
      }
    }
    if (print_basename) {
      const size_t b = Basename(p);
      PrintWithLabel(multiple ? "basename" : NULL, &p.values[b]);
    }
    if (print_extension) {
      const size_t e = Extension(p);
      if (e != not_found) {
        PrintWithLabel(multiple ? "extension" : NULL, &p.values[e]);
      } else {
        PrintWithLabel(multiple ? "extension" : NULL, "");
      }
    }
  }
}
