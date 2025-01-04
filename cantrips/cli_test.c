// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#include <stdlib.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] =
"The Turbo Encabulator. All your encabulation needs solved, with NO side-fumbling.";

static Option options[] = {
    {
        .flag = 'n',
        .description = "set Encabulator name",
        .value = { .type = TypeString }
    },
    {
        .flag = 't',
        .description = "set turbo level",
        .value = { .type = TypeInt }
    },
    {
        .flag = 'w',
        .description = "set maximum widget variance tolerance",
        .value = { .type = TypeDouble }
    },
    {
        .flag = 'x',
        .description = "explain the Turbo Encabulator",
        .value = { .type = TypeBool }
    },
};

static CLI cli = {
    .name = "turbo",
    .description = description,
    .options = {.count = COUNT(options), .options = options},
};
// clang-format on

int main(int count, char** arguments) {
  Option storage[10] = {0};
  Options os = {.capacity = COUNT(storage), .count = 0, .options = storage};
  Arguments as = {0};
  ParseCLI(&os, &as, &cli, count, arguments);

  ShowHelp(stdout, &cli);

  MustPrintf(stdout, "\nOptions parsed:\n");
  for (size_t i = 0; i < os.count; i++) {
    Option* o = &os.options[i];
    switch (o->value.type) {
      case TypeBool:
        MustPrintf(stdout, "%zu\t-%c\tbool\t%s\n", i, o->flag,
                   o->value.b ? "true" : "false");
        break;
      case TypeDouble:
        MustPrintf(stdout, "%zu\t-%c\tdouble\t%g\n", i, o->flag, o->value.d);
        break;
      case TypeInt:
        MustPrintf(stdout, "%zu\t-%c\tinteger\t%lld\n", i, o->flag, o->value.i);
        break;
      case TypeString:
        MustPrintf(stdout, "%zu\t-%c\tstring\t'%s'\n", i, o->flag, o->value.s);
        break;
    }
  }

  MustPrintf(stdout, "\nArguments:\n");
  for (size_t i = 0; i < as.count; i++) {
    MustPrintf(stdout, "%zu\t'%s'\n", i, as.arguments[i]);
  }

  const Option* explain = FindOption(&os, 'x');
  if (explain) {
    MustPrintf(stdout,
               "\nFor a number of years now, work has been proceeding in order "
               "to bring perfection to the crudely-conceived idea of a...\n");
  }
  const Option* tolerance = FindLastOption(&os, 'w');
  if (tolerance) {
    MustPrintf(stdout, "Widget variance tolerance: %g\n", tolerance->value.d);
  }
}
