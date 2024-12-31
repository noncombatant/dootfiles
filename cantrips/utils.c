// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

void noreturn PrintHelp(bool error, const char* help) {
  fputs(help, error ? stderr : stdout);
  exit(error ? EXIT_FAILURE : EXIT_SUCCESS);
}

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

void FreeRegex(regex_t** p) {
  regfree(*p);
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
    if (s[i] == c) {
      return i;
    }
  }
  return not_found;
}

size_t Format(char* result, size_t size, const char* format, ...) {
  if (size > INT_MAX || (0 != size && NULL == result) || NULL == format) {
    errno = EINVAL;
    return SIZE_MAX;
  }

  va_list arguments;
  va_start(arguments, format);
  const int count = vsnprintf(result, size, format, arguments);
  va_end(arguments);
  return count >= 0 ? (size_t)count : SIZE_MAX;
}

void MustFormat(char* result, size_t size, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const size_t count = Format(result, size, format, arguments);
  va_end(arguments);
  if (count == SIZE_MAX) {
    Die(EINVAL, "buffer too small (%zu chars) or invalid format", size);
  }
}

void MustPrintf(FILE* f, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const int count = vfprintf(f, format, arguments);
  va_end(arguments);
  if (count < 0) {
    abort();
  }
}

void Warn(int error, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  MustPrintf(stderr, format, arguments);
  va_end(arguments);
  if (error) {
    MustPrintf(stderr, ": %s", strerror(error));
  }
}

void noreturn Die(int error, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  MustPrintf(stderr, format, arguments);
  va_end(arguments);
  if (error) {
    MustPrintf(stderr, ": %s", strerror(error));
  }
  _exit(error);
}
