// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <time.h>

// Describes the type of an `OptionValue` object.
typedef enum OptionType {
  OptionTypeBool,
  OptionTypeDateTime,
  OptionTypeDouble,
  OptionTypeInt,
  OptionTypeRegex,
  OptionTypeSize,
  OptionTypeString,
} OptionType;

// The resulting value from parsing a command line option.
typedef struct OptionValue {
  OptionType type;
  bool b;
  union {
    time_t dt;
    double d;
    int64_t i;
    regex_t r;
    size_t z;
    char* s;
  };
} OptionValue;

// Describes a command line option. The `description` will be used in
// `PrintHelp`.
typedef struct Option {
  char flag;
  const char* description;
  OptionValue value;
} Option;

// A vector of `Option`s.
typedef struct Options {
  size_t count;
  Option* values;
} Options;

// A vector of C strings.
typedef struct Arguments {
  size_t count;
  char** values;
} Arguments;

// The documentation (see `PrintHelp`) and option specification for a command
// line interface.
typedef struct CLI {
  const char* name;
  const char* description;
  Options options;
} CLI;

// Prints a formatted help message, generated from the contents of `cli`. If
// `show_defaults` is true, prints the default values for the options given in
// `cli`.
void PrintHelp(FILE* output, const CLI* cli, bool show_defaults);

// Prints a formatted help message, generated from the contents of `cli`, and
// exits with status 0 (if `error` is false) or `EX_USAGE`. If `show_defaults`
// is true, prints the default values for the options given in `cli`.
noreturn void PrintHelpAndExit(const CLI* cli, bool error, bool show_defaults);

// Searches `options` for the `Option` with the matching `flag` and returns it,
// or `NULL`.
Option* FindOption(Options options, char flag);

// Searches `options` for the `Option` with the matching `flag` and returns its
// `OptionValue`, or `NULL`.
OptionValue* FindOptionValue(Options options, char flag);

// Output field separator.
extern char* OFS;

// Output record separator.
extern char* ORS;

// Sets globals `OFS`  and `ORS` to the values passed in the environment
// variables `OFS` and `ORS` if present, or to reasonable default values.
void SetSeparators(void);

// Parses the arguments and options according to the specification in `cli`,
// populates the `Options` in `cli`, and returns the remaining arguments. If
// parsing fails for any reason, calls `PrintHelpAndExit` with `error` set and
// `show_defaults` unset.
//
// As it parses the command line, `ParseCLI` will set `OptionValue.b` for each
// `Option`, even if that option’s `OptionType` is not `OptionTypeBool`. This
// allows callers to distinguish default values from values found on the command
// line.
Arguments ParseCLI(CLI* cli, int count, char** arguments);

// Prints the contents of `cli` and `arguments`.
void PrintCLI(FILE* output, const CLI* cli, const Arguments* arguments);
