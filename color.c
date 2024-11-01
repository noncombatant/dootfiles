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
static const char help[] =
"color [options] pattern color [pattern color [...]]\n"
"\n"
"-0      delimit input records with NUL instead of newline\n"
"-h      print this help message\n"
"\n"
"Patterns are case-insensitive POSIX extended regular expressions; refer to re_format(7).\n";
// clang-format on

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
    {"purple", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x31\x6d"},
};

static char normal[] = "\x1b\x28\x42\x1b\x5b\x6d";

static char* FindEscape(const char* name) {
  for (size_t i = 0; i < COUNT(colors); i++) {
    if (StringEquals(name, colors[i].name)) {
      return strdup(colors[i].escape);
    }
  }

  // Try generating the escape code if `name` is a number. We have now been
  // elected Mayor of Sillytown, and our first executive order shall be cotton
  // candy for all!
  char* end = NULL;
  const long n = strtol(name, &end, 0);
  if (end[0] != '\0') {
    return "";
  }
  char text[256];
  const int r = snprintf(text, sizeof(text), "tput setaf %ld", n);
  if (r < 0 || (size_t)r > sizeof(text)) {
    return "";
  }
  AUTO(FILE*, tput, popen(text, "r"), CloseProcess);
  if (!tput) {
    perror("popen");
    return "";
  }
  const size_t read = fread(text, 1, sizeof(text), tput);
  if (!read || read >= sizeof(text)) {
    return "";
  }
  text[read] = '\0';
  return strdup(text);
}

typedef struct Pattern {
  regex_t regex;
  Color color;
} Pattern;

typedef struct Patterns {
  size_t count;
  Pattern* patterns;
} Patterns;

static void FreePatterns(Patterns* p) {
  if (!p) {
    return;
  }
  for (size_t i = 0; i < p->count; i++) {
    regfree(&(p->patterns[i].regex));
    free(p->patterns[i].color.escape);
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
  fputs("\nYou can also use numbers as colors. Experiment and have fun!\n",
        stdout);
}

typedef struct FindResult {
  size_t pattern;
  regmatch_t match;
} FindResult;

static FindResult FindFirstMatch(const char* input, Patterns patterns) {
  FindResult result;
  memset(&result, 0, sizeof(result));
  bool found = false;
  for (size_t i = 0; i < patterns.count; i++) {
    regmatch_t match;
    const int e = regexec(&(patterns.patterns[i].regex), input, 1, &match, 0);
    if (e) {
      if (e != REG_NOMATCH) {
        PrintRegexError(e, &(patterns.patterns[i].regex));
      }
      continue;
    } else if (!found || match.rm_so < result.match.rm_so) {
      result.match = match;
      result.pattern = i;
      found = true;
    }
  }
  return result;
}

static void Colorize(Patterns patterns, char delimiter) {
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
      FindResult result = FindFirstMatch(ln, patterns);
      const regmatch_t* match = &result.match;
      if (match->rm_eo - match->rm_so) {
        const Pattern* p = &(patterns.patterns[result.pattern]);
        fwrite(ln, 1, (size_t)match->rm_so, stdout);
        fwrite(p->color.escape, 1, strlen(p->color.escape), stdout);
        fwrite(&ln[match->rm_so], 1, (size_t)(match->rm_eo - match->rm_so),
               stdout);
        fwrite(normal, 1, strlen(normal), stdout);
        length -= match->rm_eo;
        ln = &ln[match->rm_eo];
      } else {
        fwrite(ln, 1, (size_t)length, stdout);
        break;
      }
    }
    fputs("\n", stdout);
  }
}

static Patterns BuildPatterns(size_t count, char** arguments) {
  if (!count || count % 2) {
    PrintHelp(true, help);
  }

  Pattern* patterns = calloc(count / 2, sizeof(Pattern));
  for (size_t i = 0; i < count; i += 2) {
    Pattern* pattern = &patterns[i / 2];
    const int e =
        regcomp(&(pattern->regex), arguments[i], REG_EXTENDED | REG_ICASE);
    if (e) {
      PrintRegexError(e, &(pattern->regex));
      PrintHelp(true, help);
    }
    pattern->color.name = arguments[i + 1];
    pattern->color.escape = FindEscape(pattern->color.name);
  }
  return (Patterns){.count = count / 2, .patterns = patterns};
}

int main(int count, char** arguments) {
  char delimiter = '\n';
  while (true) {
    const int o = getopt(count, arguments, "0h");
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

  AUTO(Patterns, patterns, BuildPatterns((size_t)count, arguments),
       FreePatterns);
  Colorize(patterns, delimiter);
}
