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

#include "utils.h"

// clang-format off
static const char help[] =
"pathname string [...]\n"
"pathname -b string [...]\n"
"pathname -d string [...]\n"
"pathname -e string [...]\n"
"pathname -h\n"
"\n"
"With no options, prints the lexically canonicalized form(s) of the given string(s).\n"
"\n"
"-b      canonicalize the given string(s) and print their basename(s)\n"
"-d      canonicalize the given string(s) and print their dirname(s)\n"
"-e      canonicalize the given string(s) and print their file extension(s), if any\n"
"-h      print this help message\n"
"\n"
"Note that because it canonicalizes the strings first, this program may produce different results than basename(1) and dirname(1).\n";
// clang-format on

// TODO: Consider supporting printing all of -b, -d, -e, separated by $ORS.

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
  return (Bytes){.count = strlen(canonical), .bytes = canonical};
}

// Returns the index in `b.bytes` where the basename begins.
static size_t Basename(Bytes b) {
  const size_t i = LastIndex(b.bytes, b.count, path_separator);
  return i == not_found ? 0 : i + 1;
}

// Returns the length of `b.bytes`’s dirname (which will be 0, if `b.bytes` is a
// basename).
static size_t Dirname(Bytes b) {
  const size_t i = LastIndex(b.bytes, b.count, path_separator);
  return i == not_found ? 0 : i;
}

// Returns the index in the basename of `b.bytes` of the last '.', or
// `not_found`.
static size_t Extension(Bytes b) {
  const size_t basename = Basename(b);
  if (basename == not_found) {
    return LastIndex(b.bytes, b.count, '.');
  }
  const size_t dot = LastIndex(&b.bytes[basename], b.count - basename, '.');
  if (dot == not_found) {
    return dot;
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
  };
  for (size_t i = 0; i < COUNT(tests); i++) {
    AUTO(char*, copy, strdup(tests[i].pathname), FreeChar);
    const Bytes bytes = PathnameFromString(copy);
    const size_t b = Basename(bytes);
    const char* basename = &bytes.bytes[b];
    if (!StringEquals(basename, tests[i].want)) {
      MustPrintf(stderr, "Basename '%s': wanted '%s', got '%s'\n",
                 tests[i].pathname, tests[i].want, basename);
    }
  }
}

static void TestDirname() {
  Test tests[] = {
      {"/goat/bloat/../../../../../etc/passwd", "/etc"},
      {"goat/blorp/././///yow", "goat/blorp"},
      {"/goat/blorp/../../../../../", "."},
      {"", "."},
      {"/leg/foot///////////", "/leg"},
      {".", "."},
      {"/", "."},  // TODO: Should maybe/probably be "/"
      {"./", "."},
      {"../goat/../", "."},
  };
  for (size_t i = 0; i < COUNT(tests); i++) {
    AUTO(char*, copy, strdup(tests[i].pathname), FreeChar);
    const Bytes bytes = PathnameFromString(copy);
    const size_t d = Dirname(bytes);
    if (d) {
      bytes.bytes[d] = '\0';
    }
    const char* basename = d ? bytes.bytes : ".";
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
    const char* extension = e == not_found ? "" : &bytes.bytes[e];
    if (!StringEquals(extension, tests[i].want)) {
      MustPrintf(stderr, "Extension '%s': wanted '%s', got '%s'\n",
                 tests[i].pathname, tests[i].want, extension);
    }
  }
}
#endif

int main(int count, char** arguments) {
#if defined(TEST)
  TestLexicallyCanonicalizePathname();
  TestBasename();
  TestDirname();
  TestExtension();
#endif

  bool print_basename = false;
  bool print_dirname = false;
  bool print_extension = false;
  opterr = 0;
  while (true) {
    const int o = getopt(count, arguments, "bdeh");
    if (o == -1) {
      break;
    }
    switch (o) {
      case 'b':
        print_basename = true;
        break;
      case 'd':
        print_dirname = true;
        break;
      case 'e':
        print_extension = true;
        break;
      case 'h':
        PrintHelp(false, help);
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;
  const bool print_canonical =
      !print_basename && !print_dirname && !print_extension;
  if (count == 0 || print_basename + print_dirname + print_extension > 1) {
    PrintHelp(true, help);
  }

  for (int i = 0; i < count; i++) {
    AUTO(char*, copy, strdup(arguments[i]), FreeChar);
    const Bytes p = PathnameFromString(copy);
    if (print_canonical) {
      MustPrintf(stdout, "%s\n", p.bytes);
    } else if (print_basename) {
      const size_t b = Basename(p);
      MustPrintf(stdout, "%s\n", &p.bytes[b]);
    } else if (print_dirname) {
      const size_t d = Dirname(p);
      if (d) {
        p.bytes[d] = '\0';
        MustPrintf(stdout, "%s\n", p.bytes);
      } else {
        MustPrintf(stdout, ".\n");
      }
    } else if (print_extension) {
      const size_t e = Extension(p);
      if (e != not_found) {
        MustPrintf(stdout, "%s\n", &p.bytes[e]);
      }
    }
  }
}
