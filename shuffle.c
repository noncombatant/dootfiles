// This program reads in records from files, shuffles them cryptographically
// randomly, and outputs them. By default, the record is the line; use $OFS and
// $IFS to change the record delimiters.
//
// Written by Chris Palmer <https://noncombatant.org> 24 April 2010.
// SPDX-License-Identifier: MIT

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "utils.h"

static char random_file[] = "/dev/urandom";

// TODO: Optimize this
static uint64_t Random() {
  static FILE* random = NULL;

// Set the buffer to be effective but not so large that it eats up too much
// randomness.
#define buffer_size 0x100 * sizeof(uint64_t)
  static char buffer[buffer_size] = "";

  if (!random) {
    random = fopen(random_file, "rb");
    if (!random) {
      err(errno, "%s", random_file);
    }
    setbuffer(random, buffer, buffer_size);
  }

  uint64_t value;
  if (1 != fread(&value, sizeof(uint64_t), 1, random)) {
    err(1, "%s", random_file);
  }
  return value;
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
  char* fs = getenv("IFS");
  const char ifs = fs ? fs[0] : '\n';
  fs = getenv("OFS");
  const char* ofs = fs ? fs : "\n";

  if (count == 1) {
    RandomizeLines(stdin, ifs, ofs);
  }
  for (int i = 1; i < count; i++) {
    AUTO(FILE*, input, fopen(arguments[i], "rb"), CloseFile);
    if (!input) {
      warn("%s", arguments[1]);
      continue;
    }
    RandomizeLines(input, ifs, ofs);
  }
}
