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

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] =
"shuffle records of input\n"
"\n"
"    shuffle [options...] [pathnames...]\n"
"\n"
"You can shuffle either by prefixing records of a stream with a random number to be sorted with `sort`, or in memory.";

static Option options[] = {
    {
    .flag = '0',
    .description = "delimit input records with NUL instead of newline",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'm',
    .description = "shuffle in memory (uses more memory but the shuffle is faster)",
    .value = { .type = OptionTypeBool }
  },
};

static CLI cli = {
  .name = "shuffle",
  .description = description,
  .options = {.count = COUNT(options), .values = options},
};
// clang-format on

// NOTE: This function is not thread-safe and is not `fork`-safe!
static uint64_t CachingRandom() {
  // macOS' `getentropy` won't accept more than 256 bytes.
  static uint64_t r[256 / sizeof(uint64_t)];
  static size_t i = COUNT(r);

  if (i == COUNT(r)) {
#if defined(__MACH__)
    if (getentropy(&r[0], sizeof(r))) {
      Die(errno, "getentropy");
    }
#elif defined(__linux)
    if (sizeof(r) != (size_t)getrandom(&r[0], sizeof(r), GRND_NONBLOCK)) {
      Die(errno, "getrandom");
    }
#else
#error unsupported platform
#endif
    i = 0;
  }
  return r[i++];
}

// Returns `lo` ≤ `n` ≤ `hi`. Returns 0 and sets `errno` to `EDOM` if `lo` ≥
// `hi`.
//
// NOTE: This function is not thread-safe and is not `fork`-safe!
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
    uint64_t r = CachingRandom();
    r = r & hi_mask;
    if (r <= hi) {
      return lo + r;
    }
  }
}

static char* Read(FILE* input, char fs) {
  static char* record = NULL;
  static size_t capacity = 0;
  ssize_t r = getdelim(&record, &capacity, fs, input);
  if (r == -1) {
    return NULL;
  }
  record[r - 1] = '\0';
  return record;
}

typedef void Shuffler(FILE* input, char fs);

static void ShuffleStream(FILE* input, char fs) {
  while (true) {
    const char* record = Read(input, fs);
    if (!record) {
      return;
    }
    MustPrintf(stdout, "%016" PRIx64 "%s%s%s", CachingRandom(), OFS, record,
               ORS);
  }
}

// A growable array of strings.
typedef struct Records {
  size_t capacity;
  size_t count;
  char** data;
} Records;

static void Append(Records* r, char* new) {
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

static void ShuffleInMemory(FILE* input, char fs) {
  Records records = {0};
  while (true) {
    char* record = Read(input, fs);
    if (!record) {
      break;
    }
    Append(&records, strdup(record));
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

  for (size_t i = 0; i < records.count; i++) {
    MustPrintf(stdout, "%s%s", records.data[i], ORS);
  }
}

#ifdef TEST
static void TestRandomInRangeBias() {
  // We should get roughly the same # of each value. `repetitions` is high
  // enough that significant variance would indicate a problem.
#define VALUES 10
  int counts[VALUES] = {0};
  const int r = 1000000;
  const int repetitions = VALUES * r;
  const int64_t start = GetEpochNanoseconds();
  for (int i = 0; i < repetitions; i++) {
    const uint64_t n = RandomInRange(0, 9);
    counts[n]++;
  }
  const int64_t end = GetEpochNanoseconds();
  MustPrintf(stdout,
             "%d random values in %" PRId64 " ns = %" PRId64 " ns per\n",
             repetitions, end - start, (end - start) / repetitions);
  const double arbitrary_tolerance = 0.003;
  for (size_t i = 0; i < VALUES; i++) {
    const int a = abs(r - counts[i]);
    const double d = (double)a / (double)r;
    MustPrintf(stdout, "%1d  %9d  %.6f %%\n", i, counts[i], d);
    if (d > arbitrary_tolerance) {
      MustPrintf(stderr, "FAILED: bias exceeded tolerance %g\n",
                 arbitrary_tolerance);
      exit(EXIT_FAILURE);
    }
  }
#undef VALUES
}
#endif

int main(int count, char** arguments) {
#ifdef TEST
  TestRandomInRangeBias();
#endif

  Arguments as = ParseCLI(&cli, count, arguments);
  if (FindOptionValue(cli.options, 'h')->b) {
    PrintHelpAndExit(&cli, false, true);
  }
  Shuffler* shuffle =
      FindOptionValue(cli.options, 'm')->b ? ShuffleInMemory : ShuffleStream;

  char fs = FindOptionValue(cli.options, '0')->b ? '\0' : '\n';
  SetSeparators();

  if (as.count == 0) {
    shuffle(stdin, fs);
  }
  for (size_t i = 0; i < as.count; i++) {
    AUTO(FILE*, input, fopen(as.values[i], "rb"), CloseFile);
    if (!input) {
      Warn(errno, "%s", as.values[i]);
      continue;
    }
    shuffle(input, fs);
  }
}
