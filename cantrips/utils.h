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
#include <time.h>

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

#if !__has_attribute(cleanup)
#error we depend on __attribute__((cleanup))
#endif

// Declares a variable `name` of type `type`, with initial value `value`, which
// will be destroyed by `destructor` when `name` goes out of scope. See example
// destructors below.
#define AUTO(type, name, value, destructor) \
  type name __attribute__((cleanup(destructor))) = (value)

// `closedir`s `*p` if it is non-`NULL` and `Warn`s if there is any error.  See
// `AUTO`.
void CloseDir(DIR** p);

// `fclose`s `*p` if it is non-`NULL` and `Warn`s if there is any error.  See
// `AUTO`.
void CloseFile(FILE** p);

// `pclose`s `*p` if it is non-`NULL` and `Warn`s if there is any error.  See
// `AUTO`.
void CloseProcess(FILE** p);

// `free`s `*p`.  See `AUTO`.
void FreeChar(char** p);

// `closedir`s `*p` if it is non-`NULL` and `Die`s if there is any error.  See
// `AUTO`.
void MustCloseDir(DIR** p);

// `fclose`s `*p` if it is non-`NULL` and `Die`s if there is any error.  See
// `AUTO`.
void MustCloseFile(FILE** p);

// `pclose`s `*p` if it is non-`NULL` and `Die`s if there is any error.  See
// `AUTO`.
void MustCloseProcess(FILE** p);

// Returns the number of items the C array `array` has space for.
#define COUNT(array) (sizeof((array)) / sizeof((array)[0]))

// Returns the number of UTF-8 characters in the first `count` bytes of
// `string`.
size_t CountUTF8(const char* s, size_t count);

// Reports whether `a` and `b` are equal C strings.
bool StringEquals(const char* a, const char* b);

// A vector of `char`s.
typedef struct Chars {
  size_t count;
  char* values;
} Chars;

// Returns the position of the last instance of `c` in the string `s`, of
// `length` bytes, or `SIZE_MAX` to indicate not found.
size_t LastIndex(const char* s, size_t length, char c);

// Formats the string specified by `format` into `result` (see `vsnprintf`),
// writing no more than `size` characters. Always `NUL`-terminates `result`.
// Returns the number of characters that would have been written into `result`
// had there been sufficient space, or `SIZE_MAX` if an error occured.
size_t Format(char* result, size_t size, const char* format, ...)
    __attribute__((__format__(__printf__, 3, 0)));

// Like `Format`, but `Die`s if `size` is insufficient or if there was an error.
void MustFormat(char* result, size_t size, const char* format, ...)
    __attribute__((__format__(__printf__, 3, 0)));

// `fprintf`s `format` and arguments to `output`. `abort`s on error.
void MustPrintf(FILE* output, const char* format, ...)
    __attribute__((__format__(__printf__, 2, 0)));

// Prints `format` to `stderr`. If `error` is non-zero, appends an error
// message (see `strerror`).
void Warn(int error, const char* format, ...)
    __attribute__((__format__(__printf__, 2, 0)));

// Prints `format` to `stderr`. If `error` is non-zero, appends an error message
// (see `strerror`). Then `_exit`s with `error` as the status.
void noreturn Die(int error, const char* format, ...)
    __attribute__((__format__(__printf__, 2, 0)));

// A result type for `regex_t`.
typedef struct Regex {
  int error;
  regex_t value;
} Regex;

// Attempts to compile `pattern` into a `regex_t`, with `flags`. If successful,
// `Regex.error` will be 0 and `Regex.value` will be a valid RE; otherwise,
// `.error` will indicate the error.
Regex CompileRegex(const char* pattern, int flags);

// Destroys `*r`.
void FreeRegex(Regex* r);

// Prints the `regex` `error` to `stderr`.
void PrintRegexError(int error, const regex_t* regex);

// An option type for `struct tm`: `valid` reports the status of `value`.
typedef struct DateTime {
  bool valid;
  struct tm value;
} DateTime;

// Using `strptime`, attempts to parse `string` first as "%Y-%m-%d %H:%M:%S". If
// successful, returns a valid `DateTime`. Otherwise, attempts to parse `string`
// as "%Y-%m-%d". If successful, returns a valid `DateTime`. Otherwise, attempts
// to parse `string` as "%H:%M:%S". Otherwise, returns an invalid `DateTime`.
DateTime ParseDateTime(const char* string);

// Returns the number of nanoseconds that have elapsed since the POSIX Epoch.
int64_t GetEpochNanoseconds(void);

#endif
