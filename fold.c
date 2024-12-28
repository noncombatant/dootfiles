// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static const char help[] =
"fold [-w] [pathnames...]\n"
"fold -h\n"
"\n"
"-h      print this help message\n"
"-w width\n"
"        maximum line width (default: 80)\n"
"\n"
"Width can be given in any base; refer to strtol(3).\n";
// clang-format on

static char* SkipSpaces(char* line) {
  while (isspace(*line)) {
    line++;
  }
  return line;
}

static size_t FindNextSpace(const char* line) {
  size_t n = 0;
  while (!isspace(*line)) {
    n++;
    line++;
  }
  return n;
}

// TODO: Move this to utils.
static size_t GetUTF8Count(const char* s, size_t count) {
  size_t c = 0;
  for (size_t i = 0; i < count && s[i] != '\0'; i++) {
    c += (s[i] & 0xC0) != 0x80;
  }
  return c;
}

static void Format(FILE* output, FILE* input, size_t width) {
  AUTO(char*, line, NULL, FreeChar);
  size_t capacity = 0;
  while (true) {
    const ssize_t length = getline(&line, &capacity, input);
    if (length == -1) {
      return;
    }

    ssize_t chars_consumed = 0;
    char* p = line;
    size_t width_consumed = 0;
    while (chars_consumed < length) {
      char* word = SkipSpaces(p);
      chars_consumed += (word - p);
      if (*word == '\0') {
        break;
      }
      const size_t n = FindNextSpace(word);
      const size_t utf8_length = GetUTF8Count(word, n);
      if (width_consumed + utf8_length > width) {
        // const char* dash = strchr(word, '-');
        // if (dash) {
        //   const size_t d = (size_t)(dash - word) + 1;
        //   if (fwrite(word, 1, d, output) != d) {
        //     perror("fwrite");
        //     return;
        //   }
        //   word = &word[d];
        //   chars_consumed += d;
        //   width_consumed += GetUTF8Count(word, d);
        //   continue;
        // }
        fputs("\n", output);
        width_consumed = 0;
      } else if (width_consumed) {
        fputs(" ", output);
      }
      if (fwrite(word, 1, n, output) != n) {
        perror("fwrite");
        return;
      }
      p = &word[n];
      chars_consumed += n;
      if (width_consumed + utf8_length <= width) {
        width_consumed += utf8_length;
      }
    }
    fputs("\n", output);
  }
}

int main(int count, char** arguments) {
  opterr = 0;
  size_t width = 80;
  while (true) {
    const int o = getopt(count, arguments, "hw:");
    if (o == -1) {
      break;
    }
    switch (o) {
      case 'h':
        PrintHelp(false, help);
      case 'w': {
        char* end = NULL;
        width = (size_t)strtol(optarg, &end, 0);
        if (*end != '\0') {
          PrintHelp(true, help);
        }
        break;
      }
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  for (int i = 0; i < count; i++) {
    AUTO(FILE*, input, fopen(arguments[i], "r"), CloseFile);
    Format(stdout, input, width);
  }
  if (count == 0) {
    Format(stdout, stdin, width);
  }
}
