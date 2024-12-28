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
"pathname -h\n"
"\n"
"With no options, prints the lexically canonicalized form(s) of the given string(s).\n"
"\n"
"-b      canonicalize the given string(s) and prints their basename(s)\n"
"-d      canonicalize the given string(s) and prints their dirname(s)\n"
"-h      print this help message\n"
"\n"
"Note that because it canonicalizes the strings first, this program may produce different results than basename(1) and dirname(1).\n";
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
      fprintf(stderr, "Lexically '%s': wanted '%s', got '%s'\n",
              tests[i].pathname, tests[i].want, canonicalized);
    }
  }
}
#endif

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

int main(int count, char** arguments) {
  bool print_basename = false;
  bool print_dirname = false;
  opterr = 0;
  while (true) {
    const int o = getopt(count, arguments, "bdh");
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
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;
  const bool print_canonical = !print_basename && !print_dirname;
  if (count == 0 || (print_basename && print_dirname)) {
    PrintHelp(true, help);
  }

#if defined(TEST)
  TestLexicallyCanonicalizePathname();
#endif

  for (int i = 0; i < count; i++) {
    AUTO(char*, copy, strdup(arguments[i]), FreeChar);
    const Bytes p = PathnameFromString(copy);
    if (print_canonical) {
      printf("%s\n", p.bytes);
    } else if (print_basename) {
      const size_t b = Basename(p);
      printf("%s\n", &p.bytes[b]);
    } else if (print_dirname) {
      const size_t d = Dirname(p);
      if (d) {
        p.bytes[d] = '\0';
        printf("%s\n", p.bytes);
      } else {
        printf(".\n");
      }
    }
  }
}
