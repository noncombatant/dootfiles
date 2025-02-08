#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "utils.h"

static clockid_t clocks[] = {
#ifdef CLOCK_REALTIME
    CLOCK_REALTIME,
#endif
#ifdef CLOCK_REALTIME_ALARM
    CLOCK_REALTIME_ALARM,
#endif
#ifdef CLOCK_REALTIME_COARSE
    CLOCK_REALTIME_COARSE,
#endif
#ifdef CLOCK_TAI
    CLOCK_TAI,
#endif
#ifdef CLOCK_MONOTONIC
    CLOCK_MONOTONIC,
#endif
#ifdef CLOCK_MONOTONIC_COARSE
    CLOCK_MONOTONIC_COARSE,
#endif
#ifdef CLOCK_MONOTONIC_RAW
    CLOCK_MONOTONIC_RAW,
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
    CLOCK_MONOTONIC_RAW_APPROX,
#endif
#ifdef CLOCK_BOOTTIME
    CLOCK_BOOTTIME,
#endif
#ifdef CLOCK_BOOTTIME_ALARM
    CLOCK_BOOTTIME_ALARM,
#endif
#ifdef CLOCK_UPTIME_RAW
    CLOCK_UPTIME_RAW,
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
    CLOCK_UPTIME_RAW_APPROX,
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
    CLOCK_PROCESS_CPUTIME_ID,
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
    CLOCK_THREAD_CPUTIME_ID,
#endif
};

static char* clock_names[] = {
#ifdef CLOCK_REALTIME
    [CLOCK_REALTIME] = "realtime",
#endif
#ifdef CLOCK_REALTIME_ALARM
    [CLOCK_REALTIME_ALARM] = "realtime alarm",
#endif
#ifdef CLOCK_REALTIME_COARSE
    [CLOCK_REALTIME_COARSE] = "realtime coarse",
#endif
#ifdef CLOCK_TAI
    [CLOCK_TAI] = "TAI",
#endif
#ifdef CLOCK_MONOTONIC
    [CLOCK_MONOTONIC] = "monotonic",
#endif
#ifdef CLOCK_MONOTONIC_COARSE
    [CLOCK_MONOTONIC_COARSE] = "monotonic coarse",
#endif
#ifdef CLOCK_MONOTONIC_RAW
    [CLOCK_MONOTONIC_RAW] = "monotonic (raw)",
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
    [CLOCK_MONOTONIC_RAW_APPROX] = "monotonic (raw, approximate)",
#endif
#ifdef CLOCK_BOOTTIME
    [CLOCK_BOOTTIME] = "boot time",
#endif
#ifdef CLOCK_BOOTTIME_ALARM
    [CLOCK_BOOTTIME_ALARM] = "boot time alarm",
#endif
#ifdef CLOCK_UPTIME_RAW
    [CLOCK_UPTIME_RAW] = "uptime (raw)",
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
    [CLOCK_UPTIME_RAW_APPROX] = "uptime (raw, approximate)",
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
    [CLOCK_PROCESS_CPUTIME_ID] = "process CPU time",
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
    [CLOCK_THREAD_CPUTIME_ID] = "thread CPU time",
#endif
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
