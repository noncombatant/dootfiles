// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static char help[] =
"color [options] pattern color [pattern color [...]]\n"
"\n"
"-0      delimit input records with NUL instead of newline\n"
"-h      print this help message\n"
"\n"
"Patterns are case-insensitive POSIX extended regular expressions; refer to re_format(7).\n";
// clang-format on

static char delimiter = '\n';

typedef struct {
  char* name;
  char* escape;
} Color;

static Color colors[] = {
    {"black", "\x1b\x5b\x33\x30\x6d"},
    {"red", "\x1b\x5b\x33\x31\x6d"},
    {"green", "\x1b\x5b\x33\x32\x6d"},
    {"yellow", "\x1b\x5b\x33\x33\x6d"},
    {"blue", "\x1b\x5b\x33\x34\x6d"},
    {"magenta", "\x1b\x5b\x33\x35\x6d"},
    {"cyan", "\x1b\x5b\x33\x36\x6d"},
    {"lightgray", "\x1b\x5b\x33\x37\x6d"},
    {"gray", "\x1b\x5b\x33\x38\x6d"},
    {"orange", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x30\x30\x6d"},
    // {"lightcyan", "\x1b\x5b\x33\x96\x6d"},
    // {"white", "\x1b\x5b\x33\x97\x6d"},
};

static char normal[] = "\x1b\x28\x42\x1b\x5b\x6d";

static char* FindEscape(const char* name) {
  for (size_t i = 0; i < COUNT(colors); i++) {
    if (StringEquals(name, colors[i].name)) {
      return colors[i].escape;
    }
  }
  return "";
}

typedef struct {
  regex_t pattern;
  Color color;
} Pattern;

static void FreePattern(Pattern** p) {
  if (p && *p) {
    free(*p);
  }
}

static void PrintRegexError(int error, regex_t* regex) {
  char message[512] = "";
  (void)regerror(error, regex, message, sizeof(message));
  fprintf(stderr, "bad RE: %s\n", message);
}

static void PrintColors() {
  fputs("The available colors are:\n", stdout);
  for (size_t i = 0; i < COUNT(colors); i++) {
    printf("%s%s%s\n", colors[i].escape, colors[i].name, normal);
  }
}

typedef struct FindResult {
  size_t p;
  regmatch_t m;
} FindResult;

static FindResult FindFirstMatch(const char* input,
                                 Pattern* patterns,
                                 size_t count) {
  FindResult result;
  memset(&result, 0, sizeof(result));
  bool found = false;
  for (size_t i = 0; i < count; i++) {
    regmatch_t match;
    const int e = regexec(&patterns[i].pattern, input, 1, &match, 0);
    if (e) {
      if (e != REG_NOMATCH) {
        PrintRegexError(e, &patterns[i].pattern);
      }
      continue;
    } else if (!found || match.rm_so < result.m.rm_so) {
      result.m.rm_so = match.rm_so;
      result.m.rm_eo = match.rm_eo;
      result.p = i;
      found = true;
    }
  }
  return result;
}

static void Colorize(Pattern* patterns, size_t count) {
  AUTO(char*, line, NULL, FreeChar);
  size_t capacity = 0;
  while (true) {
    ssize_t length = getdelim(&line, &capacity, delimiter, stdin);
    if (length < 0) {
      return;
    }
    if (length && line[length - 1] == delimiter) {
      line[length - 1] = '\0';
      length--;
    }

    char* ln = line;
    while (length) {
      FindResult match = FindFirstMatch(ln, patterns, count);
      if (match.m.rm_eo - match.m.rm_so) {
        fwrite(ln, 1, (size_t)match.m.rm_so, stdout);
        fwrite(patterns[match.p].color.escape, 1,
               strlen(patterns[match.p].color.escape), stdout);
        fwrite(&ln[match.m.rm_so], 1, (size_t)(match.m.rm_eo - match.m.rm_so),
               stdout);
        fwrite(normal, 1, strlen(normal), stdout);
        length -= match.m.rm_eo;
        ln = &ln[match.m.rm_eo];
      } else {
        fwrite(ln, 1, (size_t)length, stdout);
        break;
      }
    }
    fputs("\n", stdout);
  }
}

static Pattern* BuildPatterns(size_t count, char** arguments) {
  if (!count || count % 2) {
    PrintHelp(true, help);
  }

  Pattern* patterns = calloc(count / 2, sizeof(Pattern));
  for (size_t i = 0; i < count; i += 2) {
    size_t p = i / 2;
    const int e =
        regcomp(&patterns[p].pattern, arguments[i], REG_EXTENDED | REG_ICASE);
    if (e) {
      PrintRegexError(e, &patterns[p].pattern);
      PrintHelp(true, help);
    }
    patterns[p].color.name = arguments[i + 1];
    patterns[p].color.escape = FindEscape(patterns[p].color.name);
  }
  return patterns;
}

int main(int count, char** arguments) {
  const int o = getopt(count, arguments, "0h");
  while (true) {
    if (o == -1) {
      break;
    }
    switch (o) {
      case '0':
        delimiter = '\0';
        break;
      case 'h':
        fputs(help, stdout);
        fputs("\n", stdout);
        PrintColors();
        return 0;
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  AUTO(Pattern*, patterns, BuildPatterns((size_t)count, arguments),
       FreePattern);
  Colorize(patterns, (size_t)count / 2);
}
