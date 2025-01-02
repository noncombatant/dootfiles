// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdnoreturn.h>

typedef enum Type {
  TypeBool,
  TypeDouble,
  TypeInt,
  TypeString,
} Type;

typedef struct Value {
  Type type;
  bool b;
  double d;
  int64_t i;
  char* s;
} Value;

typedef struct Option {
  char flag;
  const char* description;
  Value value;
} Option;

typedef struct Options {
  size_t capacity;
  size_t count;
  Option* options;
} Options;

typedef struct Arguments {
  size_t capacity;
  size_t count;
  char** arguments;
} Arguments;

typedef struct CLI {
  const char* name;
  const char* description;
  const Options options;
} CLI;

// Prints a formatted help message, generated from the contents of `h`.
void ShowHelp(FILE* output, const CLI* cli);

// Prints a formatted help message, generated from the contents of `h`, and
// exits with status 0 (if `error` is false) or `EX_USAGE`.
noreturn void ShowHelpAndExit(const CLI* cli, bool error);

// Returns the first instance of the option `flag` in `o`, or `NULL`.
Option* FindOption(const Options* o, char flag);

// Parses the `count` `arguments` according to the specification in `cli`, and
// populates `os` and `as` (which are caller-allocated). If parsing fails for
// any reason, calls `ShowHelpAndExit` with `error` set.
void ParseCLI(Options* os,
              Arguments* as,
              const CLI* cli,
              int count,
              char** arguments);
