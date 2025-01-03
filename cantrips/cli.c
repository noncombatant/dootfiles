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

void ShowHelp(FILE* output, const CLI* cli) {
  MustPrintf(output, "%s\n\n%s\n", cli->name, cli->description);

  MustPrintf(output, "\nOptions\n\n");
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

static void BuildOptString(char* result, size_t size, const Options* o) {
  size_t flag = 0;
  for (size_t i = 0; i < o->count && flag < size; i++) {
    result[flag] = o->options[i].flag;
    flag++;
    if (o->options[i].value.type != TypeBool) {
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

Option* FindOption(const Options* o, char flag) {
  Option find = {.flag = flag};
  size_t count = o->count;
  return lfind(&find, o->options, &count, sizeof(Option), CompareOption);
}

// Maximum of e.g. 127 Boolean options or 63 argument-taking options. Not even
// `ls` has that many, so this should suffice.
#define OPTSTRING_LENGTH 128

void ParseCLI(Options* os,
              Arguments* as,
              const CLI* cli,
              int count,
              char** arguments) {
  char optstring[OPTSTRING_LENGTH];
  assert(cli->options.count * 2 < sizeof(optstring));
  BuildOptString(optstring, sizeof(optstring), &(cli->options));

  opterr = 0;
  os->count = 0;
  while (os->count < os->capacity) {
    const int flag = getopt(count, arguments, optstring);
    if (flag == -1) {
      break;
    }

    const Option* o = FindOption(&(cli->options), (char)flag);
    if (!o) {
      ShowHelpAndExit(cli, true);
    }

    Option* parsed = &(os->options[os->count]);
    parsed->flag = (char)flag;
    parsed->value.type = o->value.type;
    char* end;
    switch (o->value.type) {
      case TypeBool:
        parsed->value.b = true;
        break;
      case TypeDouble:
        parsed->value.d = strtod(optarg, &end);
        if (*end != '\0') {
          ShowHelpAndExit(cli, true);
        }
        break;
      case TypeInt:
        parsed->value.i = strtoll(optarg, &end, 0);
        if (*end != '\0') {
          ShowHelpAndExit(cli, true);
        }
        break;
      case TypeString:
        parsed->value.s = strdup(optarg);
        break;
    }
    os->count++;
  }

  as->count = (size_t)(count - optind);
  as->arguments = arguments + optind;
}
