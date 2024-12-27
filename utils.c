// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void noreturn PrintHelp(bool error, const char* help) {
  fputs(help, error ? stderr : stdout);
  exit(error ? EXIT_FAILURE : EXIT_SUCCESS);
}

bool StringEquals(const char* a, const char* b) {
  return strcmp(a, b) == 0;
}

void CloseDir(DIR** p) {
  if (*p && closedir(*p)) {
    perror("closedir");
  }
}

void CloseFile(FILE** p) {
  if (*p && fclose(*p) != 0) {
    perror("fclose");
  }
}

void CloseProcess(FILE** p) {
  if (*p && pclose(*p)) {
    perror("pclose");
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
    abort();
  }
}

void MustCloseFile(FILE** p) {
  if (*p && fclose(*p) != 0) {
    abort();
  }
}

void MustCloseProcess(FILE** p) {
  if (*p && pclose(*p)) {
    abort();
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
