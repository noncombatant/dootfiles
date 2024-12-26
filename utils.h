// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#ifndef UTILS_H
#define UTILS_H

#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stdnoreturn.h>

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_attribute(cleanup)
#define AUTO(type, name, value, destructor) \
  type name __attribute__((cleanup(destructor))) = (value)
#else
#error we depend on __attribute__((cleanup))
#endif

#define COUNT(array) (sizeof((array)) / sizeof((array)[0]))

void noreturn PrintHelp(bool error, const char* help);

bool StringEquals(const char* a, const char* b);

void CloseDir(DIR** p);
void CloseFile(FILE** p);
void CloseProcess(FILE** p);
void FreeChar(char** p);
void FreeRegex(regex_t** p);
void MustCloseDir(DIR** p);
void MustCloseFile(FILE** p);
void MustCloseProcess(FILE** p);

#endif
