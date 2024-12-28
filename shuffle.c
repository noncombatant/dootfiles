// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static char help[] =
"shuffle [file...]\n"
"shuffle -h\n"
"\n"
"-h      print this help message\n";
// clang-format on

static uint64_t Random() {
  uint64_t r;
#if defined(__MACH__)
  if (getentropy(&r, sizeof(r))) {
    err(errno, "getentropy");
  }
#elif defined(__linux)
  if (sizeof(r) != (size_t)getrandom(&r, sizeof(r), GRND_NONBLOCK)) {
    err(errno, "getrandom");
  }
#else
#error unsupported platform
#endif
  return r;
}

static void RandomizeLines(FILE* input, char ifs, const char* ofs) {
  AUTO(char*, line, NULL, FreeChar);
  size_t capacity = 0;
  while (true) {
    ssize_t r = getdelim(&line, &capacity, ifs, input);
    if (-1 == r) {
      break;
    }
    line[r - 1] = '\0';
    // TODO: Consider parameterizing ORS too.
    printf("%016" PRIx64 "\t%s%s", Random(), line, ofs);
  }
}

int main(int count, char** arguments) {
  opterr = 0;
  while (true) {
    const int o = getopt(count, arguments, "h");
    if (o == -1) {
      break;
    }
    switch (o) {
      case 'h':
        PrintHelp(false, help);
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  char* fs = getenv("IFS");
  const char ifs = fs ? fs[0] : '\n';
  fs = getenv("OFS");
  const char* ofs = fs ? fs : "\n";

  if (count == 0) {
    RandomizeLines(stdin, ifs, ofs);
  }
  for (int i = 0; i < count; i++) {
    AUTO(FILE*, input, fopen(arguments[i], "rb"), CloseFile);
    if (!input) {
      warn("%s", arguments[i]);
      continue;
    }
    RandomizeLines(input, ifs, ofs);
  }
}
