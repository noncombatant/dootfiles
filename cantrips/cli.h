// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdnoreturn.h>

typedef enum OptionType {
  OptionTypeBool,
  OptionTypeDouble,
  OptionTypeInt,
  OptionTypeSize,
  OptionTypeString,
} OptionType;

typedef struct OptionValue {
  OptionType type;
  bool b;
  double d;
  int64_t i;
  size_t z;
  char* s;
} OptionValue;

typedef struct Option {
  char flag;
  const char* description;
  OptionValue value;
} Option;

typedef struct Options {
  size_t count;
  Option* options;
} Options;

typedef struct Arguments {
  size_t count;
  char** arguments;
} Arguments;

typedef struct CLI {
  const char* name;
  const char* description;
  Options options;
} CLI;

// Prints a formatted help message, generated from the contents of `cli`. If
// `show_defaults` is true, prints the default values for the options given in
// `cli`.
void ShowHelp(FILE* output, const CLI* cli, bool show_defaults);

// Prints a formatted help message, generated from the contents of `cli`, and
// exits with status 0 (if `error` is false) or `EX_USAGE`. If `show_defaults`
// is true, prints the default values for the options given in `cli`.
noreturn void ShowHelpAndExit(const CLI* cli, bool error, bool show_defaults);

// Searches `options` for the `Option` with the matching `flag` and returns it,
// or `NULL`.
Option* FindOption(const Options* options, char flag);

// Searches `options` for the `Option` with the matching `flag` and returns its
// `OptionValue`, or `NULL`.
OptionValue* FindOptionValue(const Options* options, char flag);

// Parses the arguments and options according to the specification in `cli`,
// populates the `Options` in `cli`, and returns the remaining arguments. If
// parsing fails for any reason, calls `ShowHelpAndExit` with `error` set and
// `show_defaults` unset.
//
// As it parses the command line, `ParseCLI` will set `OptionValue.b` for each
// `Option`, even if that option’s `OptionType` is not `OptionTypeBool`. This
// allows callers to distinguish default values from values found on the command
// line.
Arguments ParseCLI(CLI* cli, int count, char** arguments);

void PrintCLI(FILE* output, const CLI* cli, const Arguments* arguments);
