// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] = "fold lines of text to a maximum width\n"
"\n"
"    fold [options...] [pathnames...]";

static Option options[] = {
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'w',
    .description = "maximum line width; can be given in any base (refer to `strtol`(3))",
    .value = { .type = OptionTypeSize, .z = 80 }
  },
};

static CLI cli = {
  .name = "fold",
  .description = description,
  .options = {.count = COUNT(options), .values = options},
};
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

static void Fold(FILE* output, FILE* input, size_t width) {
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
      const size_t utf8_length = CountUTF8(word, n);
      if (width_consumed + utf8_length > width) {
        // const char* dash = strchr(word, '-');
        // if (dash) {
        //   const size_t d = (size_t)(dash - word) + 1;
        //   if (fwrite(word, 1, d, output) != d) {
        //     Warn(errno, "fwrite");
        //     return;
        //   }
        //   word = &word[d];
        //   chars_consumed += d;
        //   width_consumed += CountUTF8(word, d);
        //   continue;
        // }
        fputs("\n", output);
        width_consumed = 0;
      } else if (width_consumed) {
        fputs(" ", output);
      }
      if (fwrite(word, 1, n, output) != n) {
        Warn(errno, "fwrite");
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
  Arguments as = ParseCLI(&cli, count, arguments);
  const size_t width = FindOptionValue(cli.options, 'w')->z;
  if (FindOptionValue(cli.options, 'h')->b) {
    PrintHelpAndExit(&cli, false, true);
  }
  for (size_t i = 0; i < as.count; i++) {
    AUTO(FILE*, input, fopen(as.values[i], "r"), CloseFile);
    Fold(stdout, input, width);
  }
  if (count == 0) {
    Fold(stdout, stdin, width);
  }
}
