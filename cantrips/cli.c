// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// TODO: Rename ShowHelp to PrintHelp[AndExit] after migrating everything to
// this library.

// TODO: Add an option to show defaults.
void ShowHelp(FILE* output, const CLI* cli) {
  MustPrintf(output, "%s\n\n%s\n\nOptions\n\n", cli->name, cli->description);
  for (size_t i = 0; i < cli->options.count; i++) {
    const Option* o = &(cli->options.options[i]);
    char* t = "";
    switch (o->value.type) {
      case TypeBool:
        break;
      case TypeDouble:
        t = " floating-point";
        break;
      case TypeInt:
        t = " integer";
        break;
      case TypeString:
        t = " string";
        break;
    }
    MustPrintf(output, "-%c%s\n    %s\n", o->flag, t, o->description);
  }
}

noreturn void ShowHelpAndExit(const CLI* cli, bool error) {
  ShowHelp(error ? stderr : stdout, cli);
  exit(error ? EX_USAGE : 0);
}

static void BuildOptString(char* result, size_t size, const Options* options) {
  // TODO: Should be able to get rid of `flag` and just use `i`?
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
      ShowHelpAndExit(cli, true);
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
          ShowHelpAndExit(cli, true);
        }
        break;
      }
      case TypeInt: {
        char* end;
        v->i = strtoll(optarg, &end, 0);
        if (*end != '\0') {
          ShowHelpAndExit(cli, true);
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
