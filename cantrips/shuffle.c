// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static const char help[] =
"shuffle [file...] | sort | sed -E 's/[0-9a-f]{16}\\t//'\n"
"shuffle -m [file...]\n"
"shuffle -h\n"
"\n"
"-h      print this help message\n"
"-m      shuffle in memory (uses more memory but the shuffle is faster)\n";
// clang-format on

// TODO: Also support -0

static uint64_t Random() {
  uint64_t r;
#if defined(__MACH__)
  if (getentropy(&r, sizeof(r))) {
    Die(errno, "getentropy");
  }
#elif defined(__linux)
  if (sizeof(r) != (size_t)getrandom(&r, sizeof(r), GRND_NONBLOCK)) {
    Die(errno, "getrandom");
  }
#else
#error unsupported platform
#endif
  return r;
}

// Returns `lo` ≤ `n` ≤ `hi`. Returns 0 and sets `errno` to `EDOM` if `lo` ≥
// `hi`.
static uint64_t RandomInRange(uint64_t lo, uint64_t hi) {
  if (lo >= hi) {
    errno = EDOM;
    return 0;
  }

  hi = hi - lo;
  const uint64_t hi_order = 64 - (uint64_t)__builtin_clzl(hi);
  const uint64_t hi_mask = ~(UINT64_MAX << hi_order);

  while (true) {
    // Get a random number, find and then mask away the high-order bits that
    // make it > `hi`, and then check. Checking and retrying is the only way
    // (that I know of) to avoid introducing bias.
    uint64_t r = Random();
    r = r & hi_mask;
    if (r <= hi) {
      return lo + r;
    }
  }
}

static char* ReadRecord(FILE* input, char ifs) {
  static char* line = NULL;
  static size_t capacity = 0;
  ssize_t r = getdelim(&line, &capacity, ifs, input);
  if (r == -1) {
    return NULL;
  }
  line[r - 1] = '\0';
  return line;
}

typedef void Shuffler(FILE* input, char ifs, const char* ofs, const char* ors);

static void ShuffleStream(FILE* input,
                          char ifs,
                          const char* ofs,
                          const char* ors) {
  while (true) {
    const char* record = ReadRecord(input, ifs);
    if (!record) {
      return;
    }
    MustPrintf(stdout, "%016" PRIx64 "%s%s%s", Random(), ors, record, ofs);
  }
}

// A growable array of strings.
typedef struct Records {
  size_t capacity;
  size_t count;
  char** data;
} Records;

static void AppendRecord(Records* r, char* new) {
  assert(r->capacity >= r->count);
  if (r->capacity == r->count) {
    const size_t c = r->capacity;
    char** old_data = r->data;
    r->capacity = c ? c * 2 : 32;
    r->data = calloc(r->capacity, sizeof(char*));
    memcpy(r->data, old_data, c * sizeof(char*));
    free(old_data);
  }
  r->data[r->count] = new;
  r->count++;
}

static void ShuffleInMemory(FILE* input, char ifs, const char*, const char*) {
  Records records = {0};
  while (true) {
    char* record = ReadRecord(input, ifs);
    if (!record) {
      break;
    }
    AppendRecord(&records, strdup(record));
  }

  // Now shuffle them, Fisher-Yates stylee.
  for (size_t i = 0; i < records.count - 1; i++) {
    // j ← random integer such that i ≤ j ≤ n-1
    // exchange a[i] and a[j]
    uint64_t j = RandomInRange(i, records.count - 1);
    char* x = records.data[i];
    records.data[i] = records.data[j];
    records.data[j] = x;
  }

  MustPrintf(stdout, "%zu records\n", records.count);
  for (size_t i = 0; i < records.count; i++) {
    MustPrintf(stdout, "%s\n", records.data[i]);
  }
}

#ifdef TEST
static void TestRandomInRangeBias() {
  // We should get roughly the same # of each value. `repetitions` is high
  // enough that significant variance would indicate a problem.
#define values 10
  int counts[values] = {0};
  const int repetitions = values * 1000000;
  for (int i = 0; i < repetitions; i++) {
    const uint64_t n = RandomInRange(0, 9);
    counts[n]++;
  }
  for (size_t i = 0; i < values; i++) {
    MustPrintf(stdout, "%d: %d\n", i, counts[i]);
  }
}
#endif

int main(int count, char** arguments) {
#ifdef TEST
  TestRandomInRangeBias();
#endif

  opterr = 0;
  Shuffler* shuffle = ShuffleStream;
  while (true) {
    const int o = getopt(count, arguments, "hm");
    if (o == -1) {
      break;
    }
    switch (o) {
      case 'h':
        PrintHelp(false, help);
      case 'm':
        shuffle = ShuffleInMemory;
        break;
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  const char* IFS = getenv("IFS");
  const char ifs = IFS ? IFS[0] : '\n';
  const char* OFS = getenv("OFS");
  const char* ofs = OFS ? OFS : "\n";
  const char* ORS = getenv("ORS");
  const char* ors = ORS ? ORS : "\t";

  if (count == 0) {
    shuffle(stdin, ifs, ofs, ors);
  }
  for (int i = 0; i < count; i++) {
    AUTO(FILE*, input, fopen(arguments[i], "rb"), CloseFile);
    if (!input) {
      Warn(errno, "%s", arguments[i]);
      continue;
    }
    shuffle(input, ifs, ofs, ors);
  }
}
