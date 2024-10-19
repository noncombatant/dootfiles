// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

bool StringEquals(const char* a, const char* b) {
  return strcmp(a, b) == 0;
}

void noreturn PrintHelp(bool error, const char* help) {
  fputs(help, error ? stderr : stdout);
  exit(error);
}

void MustCloseDir(DIR** p) {
  ASSERT_P(p);
  if (*p && closedir(*p)) {
    abort();
  }
}

void CloseDir(DIR** p) {
  ASSERT_P(p);
  if (*p && closedir(*p)) {
    perror("closedir");
  }
}

void MustCloseFile(FILE** p) {
  ASSERT_P(p);
  if (*p && fclose(*p) != 0) {
    abort();
  }
}

void CloseFile(FILE** p) {
  ASSERT_P(p);
  if (*p && fclose(*p) != 0) {
    perror("fclose");
  }
}

void MustCloseProcess(FILE** p) {
  ASSERT_P(p);
  if (*p && pclose(*p)) {
    abort();
  }
}

void CloseProcess(FILE** p) {
  ASSERT_P(p);
  if (*p && pclose(*p)) {
    perror("pclose");
  }
}

void FreeChar(char** p) {
  ASSERT_P(p);
  free(*p);
}
