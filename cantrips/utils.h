// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>

static_assert(CHAR_BIT == 8, "we assume 8-bit chars");
static_assert(sizeof(char) == sizeof(int8_t),
              "we assume char is the same size as int8_t");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-logical-operand"
static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8,
              "we assume a 32- or 64-bit machine");
#pragma clang diagnostic pop
static_assert(sizeof(size_t) == sizeof(void*),
              "we assume size_t is the same size as a pointer");
static_assert(sizeof(size_t) == sizeof(intptr_t),
              "we assume size_t is the same size as intptr_t");

#ifndef __has_attribute
#error we depend on __has_attribute
#endif

#if __has_attribute(cleanup)
#define AUTO(type, name, value, destructor) \
  type name __attribute__((cleanup(destructor))) = (value)
#else
#error we depend on __attribute__((cleanup))
#endif

#define COUNT(array) (sizeof((array)) / sizeof((array)[0]))

void noreturn PrintHelp(bool error, const char* help);

// Returns the number of UTF-8 characters in the first `count` bytes of
// `string`.
size_t CountUTF8(const char* s, size_t count);

bool StringEquals(const char* a, const char* b);

void CloseDir(DIR** p);
void CloseFile(FILE** p);
void CloseProcess(FILE** p);
void FreeChar(char** p);
void MustCloseDir(DIR** p);
void MustCloseFile(FILE** p);
void MustCloseProcess(FILE** p);

typedef struct Bytes {
  size_t count;
  char* bytes;
} Bytes;

extern const size_t not_found;

// Returns the position of the last instance of `c` in the string `s`, of
// `length` bytes.
size_t LastIndex(const char* s, size_t length, char c);

__attribute__((__format__(__printf__, 3, 0))) size_t Format(char* result,
                                                            size_t size,
                                                            const char* format,
                                                            ...);

__attribute__((__format__(__printf__, 3, 0))) void
MustFormat(char* result, size_t size, const char* format, ...);

__attribute__((__format__(__printf__, 2, 0))) void
MustPrintf(FILE* f, const char* format, ...);

__attribute__((__format__(__printf__, 2, 0))) void Warn(int error,
                                                        const char* format,
                                                        ...);

__attribute__((__format__(__printf__, 2, 0))) void noreturn
Die(int error, const char* format, ...);

typedef struct Regex {
  int error;
  regex_t regex;
} Regex;

Regex CompileRegex(const char* pattern, int flags);
void FreeRegex(Regex* r);
void PrintRegexError(int error, const regex_t* regex);

#endif
