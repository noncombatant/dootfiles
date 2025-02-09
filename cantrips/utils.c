// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

size_t CountUTF8(const char* s, size_t count) {
  size_t c = 0;
  for (size_t i = 0; i < count && s[i] != '\0'; i++) {
    c += (s[i] & 0xC0) != 0x80;
  }
  return c;
}

bool StringEquals(const char* a, const char* b) {
  return strcmp(a, b) == 0;
}

void CloseDir(DIR** p) {
  if (*p && closedir(*p)) {
    Warn(errno, "closedir");
  }
}

void CloseFile(FILE** p) {
  if (*p && fclose(*p)) {
    Warn(errno, "fclose");
  }
}

void CloseProcess(FILE** p) {
  if (*p && pclose(*p)) {
    Warn(errno, "pclose");
  }
}

void FreeChar(char** p) {
  free(*p);
}

void MustCloseDir(DIR** p) {
  if (*p && closedir(*p)) {
    Die(errno, "closedir");
  }
}

void MustCloseFile(FILE** p) {
  if (*p && fclose(*p)) {
    Die(errno, "fclose");
  }
}

void MustCloseProcess(FILE** p) {
  if (*p && pclose(*p)) {
    Die(errno, "pclose");
  }
}

const size_t not_found = SIZE_MAX;

size_t LastIndex(const char* s, size_t length, char c) {
  for (size_t i = length; i > 0; i--) {
    const size_t j = i - 1;
    if (s[j] == c) {
      return j;
    }
  }
  return not_found;
}

__attribute__((__format__(__printf__, 3, 0))) static size_t
VFormat(char* result, size_t size, const char* format, va_list arguments) {
  if (size > INT_MAX || (0 != size && NULL == result) || NULL == format) {
    errno = EINVAL;
    return SIZE_MAX;
  }
  const int count = vsnprintf(result, size, format, arguments);
  return count >= 0 ? (size_t)count : SIZE_MAX;
}

__attribute__((__format__(__printf__, 2, 0))) static void
VMustPrintf(FILE* f, const char* format, va_list arguments) {
  const int count = vfprintf(f, format, arguments);
  if (count < 0) {
    abort();
  }
}

size_t Format(char* result, size_t size, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const size_t count = VFormat(result, size, format, arguments);
  va_end(arguments);
  return count;
}

void MustFormat(char* result, size_t size, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const size_t count = VFormat(result, size, format, arguments);
  va_end(arguments);
  if (count == SIZE_MAX) {
    MustPrintf(stderr, "buffer too small (%zu chars) or invalid format", size);
  }
}

void MustPrintf(FILE* f, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  VMustPrintf(f, format, arguments);
  va_end(arguments);
}

void Warn(int error, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  VMustPrintf(stderr, format, arguments);
  va_end(arguments);
  if (error) {
    MustPrintf(stderr, ": %s\n", strerror(error));
  }
}

void noreturn Die(int error, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  VMustPrintf(stderr, format, arguments);
  va_end(arguments);
  if (error) {
    MustPrintf(stderr, ": %s\n", strerror(error));
  }
  _exit(error);
}

Regex CompileRegex(const char* pattern, int flags) {
  Regex status = {0};
  status.error = regcomp(&status.value, pattern, flags);
  return status;
}

void FreeRegex(Regex* r) {
  if (r && r->error == 0) {
    regfree(&r->value);
  }
}

void PrintRegexError(int error, const regex_t* regex) {
  char message[512] = {0};
  (void)regerror(error, regex, message, sizeof(message));
  MustPrintf(stderr, "bad RE: %s\n", message);
}

DateTime ParseDateTime(const char* string) {
  DateTime dt = {0};
  if (strptime(string, "%Y-%m-%d %H:%M:%S", &dt.value)) {
    dt.valid = true;
    return dt;
  }

  if (strptime(string, "%Y-%m-%d", &dt.value)) {
    dt.valid = true;
    return dt;
  }

  const time_t t = time(NULL);
  dt.value = *localtime(&t);
  if (strptime(string, "%H:%M:%S", &dt.value)) {
    dt.valid = true;
    return dt;
  }
  return (DateTime){0};
}

int64_t GetEpochNanoseconds(void) {
  struct timespec time;
  if (clock_gettime(CLOCK_REALTIME, &time)) {
    return 0;
  }
  return (time.tv_sec * 1000000000LL) + time.tv_nsec;
}
