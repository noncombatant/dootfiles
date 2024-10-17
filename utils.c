// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

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

void CloseDirOrDie(DIR** p) {
  ASSERT_P(p);
  if (*p && closedir(*p)) {
    abort();
  }
}

void CloseDirOrWarn(DIR** p) {
  ASSERT_P(p);
  if (*p && closedir(*p)) {
    perror("closedir");
  }
}

void CloseFileOrDie(FILE** p) {
  ASSERT_P(p);
  if (*p && fclose(*p) != 0) {
    abort();
  }
}

void CloseFileOrWarn(FILE** p) {
  ASSERT_P(p);
  if (*p && fclose(*p) != 0) {
    perror("fclose");
  }
}

void FreeChar(char** p) {
  ASSERT_P(p);
  free(*p);
}
