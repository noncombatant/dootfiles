#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "utils.h"

static clockid_t clocks[] = {
    CLOCK_REALTIME,           CLOCK_MONOTONIC,
    CLOCK_MONOTONIC_RAW,      CLOCK_MONOTONIC_RAW_APPROX,
    CLOCK_UPTIME_RAW,         CLOCK_UPTIME_RAW_APPROX,
    CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID,
};

static char* clock_names[] = {
    [CLOCK_REALTIME] = "realtime",
    [CLOCK_MONOTONIC] = "monotonic",
    [CLOCK_MONOTONIC_RAW] = "monotonic (raw)",
    [CLOCK_MONOTONIC_RAW_APPROX] = "monotonic (raw, approximate)",
    [CLOCK_UPTIME_RAW] = "uptime (raw)",
    [CLOCK_UPTIME_RAW_APPROX] = "uptime (raw, approximate)",
    [CLOCK_PROCESS_CPUTIME_ID] = "process CPU time",
    [CLOCK_THREAD_CPUTIME_ID] = "thread CPU time",
};

int main() {
  for (size_t i = 0; i < COUNT(clocks); i++) {
    struct timespec t = {0};
    const clockid_t c = clocks[i];
    const char* n = clock_names[c];
    if (clock_gettime(c, &t)) {
      Warn(errno, "%s", n);
      continue;
    }
    MustPrintf(stdout, "%12ld.%09ld\t%s\n", t.tv_sec, t.tv_nsec, n);
  }
}
