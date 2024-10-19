// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdnoreturn.h>

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#define ASSERT_P(p) assert((p))

#define COUNT(array) (sizeof((array)) / sizeof((array)[0]))

void noreturn PrintHelp(bool error, const char* help);

bool StringEquals(const char* a, const char* b);

#if __has_attribute(cleanup)
#define AUTO(type, name, value, destructor) \
  type name __attribute__((cleanup(destructor))) = (value)
#else
#error we depend on __attribute__((cleanup))
#endif

void CloseDirOrDie(DIR** p);
void CloseDirOrWarn(DIR** p);
void CloseFileOrDie(FILE** p);
void CloseFileOrWarn(FILE** p);

void FreeChar(char** p);

#endif
