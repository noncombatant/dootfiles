// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

void PrintHelp(FILE* output, const CLI* cli, bool show_defaults) {
  MustPrintf(output, "%s — %s\n\nOptions\n\n", cli->name, cli->description);
  for (size_t i = 0; i < cli->options.count; i++) {
    const Option* o = &(cli->options.values[i]);
    MustPrintf(output, "-%c", o->flag);
    switch (o->value.type) {
      case OptionTypeBool:
        break;
      case OptionTypeDateTime:
        MustPrintf(output, " date-time");
        break;
      case OptionTypeDouble:
        MustPrintf(output, " floating-point");
        if (show_defaults) {
          MustPrintf(output, " (default: %g)", o->value.d);
        }
        break;
      case OptionTypeInt:
        MustPrintf(output, " integer");
        if (show_defaults) {
          MustPrintf(output, " (default: %" PRId64 ")", o->value.i);
        }
        break;
      case OptionTypeRegex:
        MustPrintf(output, "regular-expression");
        break;
      case OptionTypeSize:
        MustPrintf(output, " size");
        if (show_defaults) {
          MustPrintf(output, " (default: %zu)", o->value.z);
        }
        break;
      case OptionTypeString:
        MustPrintf(output, " string");
        if (show_defaults) {
          MustPrintf(output, " (default: \"%s\")", o->value.s);
        }
        break;
    }
    MustPrintf(output, "\n    %s\n", o->description);
  }
}

noreturn void PrintHelpAndExit(const CLI* cli, bool error, bool show_defaults) {
  PrintHelp(error ? stderr : stdout, cli, show_defaults);
  exit(error ? EX_USAGE : 0);
}

static void BuildOptString(char* result, size_t size, Options options) {
  size_t flag = 0;
  for (size_t i = 0; i < options.count && flag < size; i++) {
    const Option* o = &(options.values[i]);
    result[flag] = o->flag;
    flag++;
    if (o->value.type != OptionTypeBool) {
      result[flag] = ':';
      flag++;
    }
  }
  result[flag] = '\0';
}

Option* FindOption(Options options, char flag) {
  for (size_t i = 0; i < options.count; i++) {
    Option* o = &options.values[i];
    if (o->flag == flag) {
      return o;
    }
  }
  return NULL;
}

OptionValue* FindOptionValue(Options options, char flag) {
  Option* o = FindOption(options, flag);
  return o ? &o->value : NULL;
}

// Maximum of e.g. 127 Boolean options or 63 argument-taking options. Not even
// `ls` has that many, so this should suffice.
#define OPTSTRING_LENGTH 128

Arguments ParseCLI(CLI* cli, int count, char** arguments) {
  char optstring[OPTSTRING_LENGTH];
  assert(cli->options.count * 2 < sizeof(optstring));
  BuildOptString(optstring, sizeof(optstring), cli->options);

  opterr = 0;
  for (int i = 1; i < count; i++) {
    const int flag = getopt(count, arguments, optstring);
    if (flag == -1) {
      break;
    }

    Option* o = FindOption(cli->options, (char)flag);
    if (!o) {
      PrintHelpAndExit(cli, true, false);
    }

    OptionValue* v = &(o->value);
    switch (v->type) {
      case OptionTypeBool:
        v->b = true;
        break;
      case OptionTypeDateTime: {
        DateTime dt = ParseDateTime(optarg);
        if (!dt.has_value) {
          PrintHelpAndExit(cli, true, false);
        }
        v->dt = mktime(&dt.value);
        v->b = true;
        break;
      }
      case OptionTypeDouble: {
        char* end;
        v->d = strtod(optarg, &end);
        if (*end != '\0') {
          PrintHelpAndExit(cli, true, false);
        }
        v->b = true;
        break;
      }
      case OptionTypeInt: {
        char* end;
        v->i = strtoll(optarg, &end, 0);
        if (*end != '\0') {
          PrintHelpAndExit(cli, true, false);
        }
        v->b = true;
        break;
      }
      case OptionTypeRegex: {
        Regex r = CompileRegex(optarg, REG_EXTENDED | REG_ICASE);
        if (r.error) {
          PrintRegexError(r.error, &r.regex);
          exit(EXIT_FAILURE);
        }
        v->r = r.regex;  // Yep; copy.
        v->b = true;
        break;
      }
      case OptionTypeSize: {
        char* end;
        if (sizeof(size_t) == sizeof(unsigned long long)) {
          v->z = strtoull(optarg, &end, 0);
        } else {
          static_assert(sizeof(size_t) == sizeof(unsigned long),
                        "don't know how to parse size_t");
          v->z = strtoul(optarg, &end, 0);
        }
        if (*end != '\0') {
          PrintHelpAndExit(cli, true, false);
        }
        v->b = true;
        break;
      }
      case OptionTypeString:
        v->s = strdup(optarg);
        v->b = true;
        break;
    }
  }

  return (Arguments){.count = (size_t)(count - optind),
                     .values = arguments + optind};
}

void PrintCLI(FILE* output, const CLI* cli, const Arguments* arguments) {
  MustPrintf(output, "\nOptions parsed:\n");
  const Options* os = &cli->options;
  for (size_t i = 0; i < os->count; i++) {
    Option* o = &os->values[i];
    switch (o->value.type) {
      case OptionTypeBool:
        MustPrintf(output, "%zu\t-%c\tbool\t%s\n", i, o->flag,
                   o->value.b ? "true" : "false");
        break;
      case OptionTypeDateTime:
        MustPrintf(output, "%zu\t-%c\tdate-time\t%ld\n", i, o->flag,
                   o->value.dt);
        break;
      case OptionTypeDouble:
        MustPrintf(output, "%zu\t-%c\tdouble\t%g\n", i, o->flag, o->value.d);
        break;
      case OptionTypeInt:
        MustPrintf(output, "%zu\t-%c\tinteger\t%lld\n", i, o->flag, o->value.i);
        break;
      case OptionTypeRegex:
        MustPrintf(output, "%zu\t-%c\tregular-expression\t%p\n", i, o->flag,
                   (void*)&o->value.r);
        break;
      case OptionTypeSize:
        MustPrintf(output, "%zu\t-%c\tsize\t%zu\n", i, o->flag, o->value.z);
        break;
      case OptionTypeString:
        MustPrintf(output, "%zu\t-%c\tstring\t'%s'\n", i, o->flag, o->value.s);
        break;
    }
  }

  MustPrintf(output, "\nArguments:\n");
  for (size_t i = 0; i < arguments->count; i++) {
    MustPrintf(output, "%zu\t'%s'\n", i, arguments->values[i]);
  }
}
