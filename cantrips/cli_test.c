// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#include <stdlib.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] =
"print properties of the Turbo Encabulator\n"
"\n"
"The Turbo Encabulator. All your encabulation needs solved, with NO side-fumbling.";

static Option options[] = {
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool, .b = false },
  },
  {
    .flag = 'n',
    .description = "set Encabulator name",
    .value = { .type = OptionTypeString, .s = "" }
  },
  {
    .flag = 't',
    .description = "set turbo level",
    .value = { .type = OptionTypeInt }
  },
  {
    .flag = 'w',
    .description = "set maximum widget variance tolerance",
    .value = { .type = OptionTypeDouble }
  },
  {
    .flag = 'x',
    .description = "explain the Turbo Encabulator",
    .value = { .type = OptionTypeBool }
  },
};

static CLI cli = {
    .name = "turbo",
    .description = description,
    .options = {.count = COUNT(options), .options = options},
};
// clang-format on

int main(int count, char** arguments) {
  Arguments as = ParseCLI(&cli, count, arguments);
  if (FindOptionValue(&cli.options, 'h')->b) {
    PrintHelp(stdout, &cli, true);
  }

  PrintCLI(stdout, &cli, &as);

  if (FindOptionValue(&cli.options, 'x')->b) {
    MustPrintf(stdout,
               "\nFor a number of years now, work has been proceeding in order "
               "to bring perfection to the crudely-conceived idea of a...\n");
  }
  MustPrintf(stdout, "Widget variance tolerance: %g\n",
             FindOptionValue(&cli.options, 'w')->d);
}
