// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// TODO: Rename ShowHelp to PrintHelp[AndExit] after migrating everything to
// this library.

void ShowHelp(FILE* output, const CLI* cli, bool show_defaults) {
  MustPrintf(output, "%s\n\n%s\n\nOptions\n\n", cli->name, cli->description);
  for (size_t i = 0; i < cli->options.count; i++) {
    const Option* o = &(cli->options.options[i]);
    MustPrintf(output, "-%c", o->flag);
    switch (o->value.type) {
      case TypeBool:
        break;
      case TypeDouble:
        MustPrintf(output, " floating-point");
        if (show_defaults) {
          MustPrintf(output, " (default: %g)", o->value.d);
        }
        break;
      case TypeInt:
        MustPrintf(output, " integer");
        if (show_defaults) {
          MustPrintf(output, " (default: %" PRId64 ")", o->value.i);
        }
        break;
      case TypeString:
        MustPrintf(output, " string");
        if (show_defaults) {
          MustPrintf(output, " (default: \"%s\")", o->value.s);
        }
        break;
    }
    MustPrintf(output, "\n    %s\n", o->description);
  }
}

noreturn void ShowHelpAndExit(const CLI* cli, bool error, bool show_defaults) {
  ShowHelp(error ? stderr : stdout, cli, show_defaults);
  exit(error ? EX_USAGE : 0);
}

static void BuildOptString(char* result, size_t size, const Options* options) {
  size_t flag = 0;
  for (size_t i = 0; i < options->count && flag < size; i++) {
    const Option* o = &(options->options[i]);
    result[flag] = o->flag;
    flag++;
    if (o->value.type != TypeBool) {
      result[flag] = ':';
      flag++;
    }
  }
  result[flag] = '\0';
}

static int CompareOption(const void* a, const void* b) {
  const Option* aa = a;
  const Option* bb = b;
  if (aa->flag < bb->flag) {
    return -1;
  } else if (aa->flag > bb->flag) {
    return 1;
  }
  return 0;
}

Option* FindOption(const Options* options, char flag) {
  Option find = {.flag = flag};
  size_t count = options->count;
  return lfind(&find, options->options, &count, sizeof(Option), CompareOption);
}

Value* FindOptionValue(const Options* options, char flag) {
  Option* o = FindOption(options, flag);
  return o ? &o->value : NULL;
}

// Maximum of e.g. 127 Boolean options or 63 argument-taking options. Not even
// `ls` has that many, so this should suffice.
#define OPTSTRING_LENGTH 128

Arguments ParseCLI(CLI* cli, int count, char** arguments) {
  char optstring[OPTSTRING_LENGTH];
  assert(cli->options.count * 2 < sizeof(optstring));
  Options* options = &cli->options;
  BuildOptString(optstring, sizeof(optstring), options);

  opterr = 0;
  for (int i = 1; i < count; i++) {
    const int flag = getopt(count, arguments, optstring);
    if (flag == -1) {
      break;
    }

    Option* o = FindOption(options, (char)flag);
    if (!o) {
      ShowHelpAndExit(cli, true, false);
    }

    Value* v = &(o->value);
    switch (v->type) {
      case TypeBool:
        v->b = true;
        break;
      case TypeDouble: {
        char* end;
        v->d = strtod(optarg, &end);
        if (*end != '\0') {
          ShowHelpAndExit(cli, true, false);
        }
        break;
      }
      case TypeInt: {
        char* end;
        v->i = strtoll(optarg, &end, 0);
        if (*end != '\0') {
          ShowHelpAndExit(cli, true, false);
        }
        break;
      }
      case TypeString:
        v->s = strdup(optarg);
        break;
    }
  }

  return (Arguments){.count = (size_t)(count - optind),
                     .arguments = arguments + optind};
}

void PrintCLI(FILE* output, const CLI* cli, const Arguments* arguments) {
  MustPrintf(output, "\nOptions parsed:\n");
  const Options* os = &cli->options;
  for (size_t i = 0; i < os->count; i++) {
    Option* o = &os->options[i];
    switch (o->value.type) {
      case TypeBool:
        MustPrintf(output, "%zu\t-%c\tbool\t%s\n", i, o->flag,
                   o->value.b ? "true" : "false");
        break;
      case TypeDouble:
        MustPrintf(output, "%zu\t-%c\tdouble\t%g\n", i, o->flag, o->value.d);
        break;
      case TypeInt:
        MustPrintf(output, "%zu\t-%c\tinteger\t%lld\n", i, o->flag, o->value.i);
        break;
      case TypeString:
        MustPrintf(output, "%zu\t-%c\tstring\t'%s'\n", i, o->flag, o->value.s);
        break;
    }
  }

  MustPrintf(output, "\nArguments:\n");
  for (size_t i = 0; i < arguments->count; i++) {
    MustPrintf(output, "%zu\t'%s'\n", i, arguments->arguments[i]);
  }
}
