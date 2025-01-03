// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <regex.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static const char help[] =
"color [options] pattern color [pattern color [...]]\n"
"\n"
"-0      delimit input records with NUL instead of newline\n"
"-h      print this help message\n"
"-x      print help and extended color set\n"
"\n"
"Patterns are case-insensitive POSIX extended regular expressions; refer to re_format(7).\n";
// clang-format on

typedef struct Color {
  const char* name;
  const char* escape;
} Color;

static Color colors[] = {
    {"black", "\x1b\x5b\x33\x30\x6d"}, {"red", "\x1b\x5b\x33\x31\x6d"},
    {"green", "\x1b\x5b\x33\x32\x6d"}, {"yellow", "\x1b\x5b\x33\x33\x6d"},
    {"blue", "\x1b\x5b\x33\x34\x6d"},  {"magenta", "\x1b\x5b\x33\x35\x6d"},
    {"cyan", "\x1b\x5b\x33\x36\x6d"},  {"white", "\x1b\x5b\x33\x37\x6d"},
};

static Color extended_colors[] = {
    {"0", "\x1b\x5b\x33\x30\x6d"},
    {"1", "\x1b\x5b\x33\x31\x6d"},
    {"2", "\x1b\x5b\x33\x32\x6d"},
    {"3", "\x1b\x5b\x33\x33\x6d"},
    {"4", "\x1b\x5b\x33\x34\x6d"},
    {"5", "\x1b\x5b\x33\x35\x6d"},
    {"6", "\x1b\x5b\x33\x36\x6d"},
    {"7", "\x1b\x5b\x33\x37\x6d"},
    {"8", "\x1b\x5b\x39\x30\x6d"},
    {"9", "\x1b\x5b\x39\x31\x6d"},
    {"10", "\x1b\x5b\x39\x32\x6d"},
    {"11", "\x1b\x5b\x39\x33\x6d"},
    {"12", "\x1b\x5b\x39\x34\x6d"},
    {"13", "\x1b\x5b\x39\x35\x6d"},
    {"14", "\x1b\x5b\x39\x36\x6d"},
    {"15", "\x1b\x5b\x39\x37\x6d"},
    {"16", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x6d"},
    {"17", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x6d"},
    {"18", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x6d"},
    {"19", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x6d"},
    {"20", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x6d"},
    {"21", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x6d"},
    {"22", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x6d"},
    {"23", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x6d"},
    {"24", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x6d"},
    {"25", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x6d"},
    {"26", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x36\x6d"},
    {"27", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x37\x6d"},
    {"28", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x38\x6d"},
    {"29", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x39\x6d"},
    {"30", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x30\x6d"},
    {"31", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x31\x6d"},
    {"32", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x32\x6d"},
    {"33", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x33\x6d"},
    {"34", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x34\x6d"},
    {"35", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x35\x6d"},
    {"36", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x36\x6d"},
    {"37", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x37\x6d"},
    {"38", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x38\x6d"},
    {"39", "\x1b\x5b\x33\x38\x3b\x35\x3b\x33\x39\x6d"},
    {"40", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x30\x6d"},
    {"41", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x31\x6d"},
    {"42", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x32\x6d"},
    {"43", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x33\x6d"},
    {"44", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x34\x6d"},
    {"45", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x35\x6d"},
    {"46", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x36\x6d"},
    {"47", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x37\x6d"},
    {"48", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x38\x6d"},
    {"49", "\x1b\x5b\x33\x38\x3b\x35\x3b\x34\x39\x6d"},
    {"50", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x30\x6d"},
    {"51", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x31\x6d"},
    {"52", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x32\x6d"},
    {"53", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x33\x6d"},
    {"54", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x34\x6d"},
    {"55", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x35\x6d"},
    {"56", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x36\x6d"},
    {"57", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x37\x6d"},
    {"58", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x38\x6d"},
    {"59", "\x1b\x5b\x33\x38\x3b\x35\x3b\x35\x39\x6d"},
    {"60", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x30\x6d"},
    {"61", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x31\x6d"},
    {"62", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x32\x6d"},
    {"63", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x33\x6d"},
    {"64", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x34\x6d"},
    {"65", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x35\x6d"},
    {"66", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x36\x6d"},
    {"67", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x37\x6d"},
    {"68", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x38\x6d"},
    {"69", "\x1b\x5b\x33\x38\x3b\x35\x3b\x36\x39\x6d"},
    {"70", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x30\x6d"},
    {"71", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x31\x6d"},
    {"72", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x32\x6d"},
    {"73", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x33\x6d"},
    {"74", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x34\x6d"},
    {"75", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x35\x6d"},
    {"76", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x36\x6d"},
    {"77", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x37\x6d"},
    {"78", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x38\x6d"},
    {"79", "\x1b\x5b\x33\x38\x3b\x35\x3b\x37\x39\x6d"},
    {"80", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x30\x6d"},
    {"81", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x31\x6d"},
    {"82", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x32\x6d"},
    {"83", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x33\x6d"},
    {"84", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x34\x6d"},
    {"85", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x35\x6d"},
    {"86", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x36\x6d"},
    {"87", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x37\x6d"},
    {"88", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x38\x6d"},
    {"89", "\x1b\x5b\x33\x38\x3b\x35\x3b\x38\x39\x6d"},
    {"90", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x30\x6d"},
    {"91", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x31\x6d"},
    {"92", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x32\x6d"},
    {"93", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x33\x6d"},
    {"94", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x34\x6d"},
    {"95", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x35\x6d"},
    {"96", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x36\x6d"},
    {"97", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x37\x6d"},
    {"98", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x38\x6d"},
    {"99", "\x1b\x5b\x33\x38\x3b\x35\x3b\x39\x39\x6d"},
    {"100", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x30\x6d"},
    {"101", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x31\x6d"},
    {"102", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x32\x6d"},
    {"103", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x33\x6d"},
    {"104", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x34\x6d"},
    {"105", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x35\x6d"},
    {"106", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x36\x6d"},
    {"107", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x37\x6d"},
    {"108", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x38\x6d"},
    {"109", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x30\x39\x6d"},
    {"110", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x30\x6d"},
    {"111", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x31\x6d"},
    {"112", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x32\x6d"},
    {"113", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x33\x6d"},
    {"114", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x34\x6d"},
    {"115", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x35\x6d"},
    {"116", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x36\x6d"},
    {"117", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x37\x6d"},
    {"118", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x38\x6d"},
    {"119", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x31\x39\x6d"},
    {"120", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x30\x6d"},
    {"121", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x31\x6d"},
    {"122", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x32\x6d"},
    {"123", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x33\x6d"},
    {"124", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x34\x6d"},
    {"125", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x35\x6d"},
    {"126", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x36\x6d"},
    {"127", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x37\x6d"},
    {"128", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x38\x6d"},
    {"129", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x32\x39\x6d"},
    {"130", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x30\x6d"},
    {"131", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x31\x6d"},
    {"132", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x32\x6d"},
    {"133", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x33\x6d"},
    {"134", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x34\x6d"},
    {"135", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x35\x6d"},
    {"136", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x36\x6d"},
    {"137", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x37\x6d"},
    {"138", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x38\x6d"},
    {"139", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x33\x39\x6d"},
    {"140", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x30\x6d"},
    {"141", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x31\x6d"},
    {"142", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x32\x6d"},
    {"143", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x33\x6d"},
    {"144", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x34\x6d"},
    {"145", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x35\x6d"},
    {"146", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x36\x6d"},
    {"147", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x37\x6d"},
    {"148", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x38\x6d"},
    {"149", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x34\x39\x6d"},
    {"150", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x30\x6d"},
    {"151", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x31\x6d"},
    {"152", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x32\x6d"},
    {"153", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x33\x6d"},
    {"154", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x34\x6d"},
    {"155", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x35\x6d"},
    {"156", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x36\x6d"},
    {"157", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x37\x6d"},
    {"158", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x38\x6d"},
    {"159", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x35\x39\x6d"},
    {"160", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x30\x6d"},
    {"161", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x31\x6d"},
    {"162", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x32\x6d"},
    {"163", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x33\x6d"},
    {"164", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x34\x6d"},
    {"165", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x35\x6d"},
    {"166", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x36\x6d"},
    {"167", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x37\x6d"},
    {"168", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x38\x6d"},
    {"169", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x36\x39\x6d"},
    {"170", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x30\x6d"},
    {"171", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x31\x6d"},
    {"172", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x32\x6d"},
    {"173", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x33\x6d"},
    {"174", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x34\x6d"},
    {"175", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x35\x6d"},
    {"176", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x36\x6d"},
    {"177", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x37\x6d"},
    {"178", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x38\x6d"},
    {"179", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x37\x39\x6d"},
    {"180", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x30\x6d"},
    {"181", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x31\x6d"},
    {"182", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x32\x6d"},
    {"183", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x33\x6d"},
    {"184", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x34\x6d"},
    {"185", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x35\x6d"},
    {"186", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x36\x6d"},
    {"187", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x37\x6d"},
    {"188", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x38\x6d"},
    {"189", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x38\x39\x6d"},
    {"190", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x30\x6d"},
    {"191", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x31\x6d"},
    {"192", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x32\x6d"},
    {"193", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x33\x6d"},
    {"194", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x34\x6d"},
    {"195", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x35\x6d"},
    {"196", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x36\x6d"},
    {"197", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x37\x6d"},
    {"198", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x38\x6d"},
    {"199", "\x1b\x5b\x33\x38\x3b\x35\x3b\x31\x39\x39\x6d"},
    {"200", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x30\x6d"},
    {"201", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x31\x6d"},
    {"202", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x32\x6d"},
    {"203", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x33\x6d"},
    {"204", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x34\x6d"},
    {"205", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x35\x6d"},
    {"206", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x36\x6d"},
    {"207", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x37\x6d"},
    {"208", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x38\x6d"},
    {"209", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x30\x39\x6d"},
    {"210", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x30\x6d"},
    {"211", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x31\x6d"},
    {"212", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x32\x6d"},
    {"213", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x33\x6d"},
    {"214", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x34\x6d"},
    {"215", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x35\x6d"},
    {"216", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x36\x6d"},
    {"217", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x37\x6d"},
    {"218", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x38\x6d"},
    {"219", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x31\x39\x6d"},
    {"220", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x30\x6d"},
    {"221", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x31\x6d"},
    {"222", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x32\x6d"},
    {"223", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x33\x6d"},
    {"224", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x34\x6d"},
    {"225", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x35\x6d"},
    {"226", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x36\x6d"},
    {"227", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x37\x6d"},
    {"228", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x38\x6d"},
    {"229", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x32\x39\x6d"},
    {"230", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x30\x6d"},
    {"231", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x31\x6d"},
    {"232", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x32\x6d"},
    {"233", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x33\x6d"},
    {"234", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x34\x6d"},
    {"235", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x35\x6d"},
    {"236", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x36\x6d"},
    {"237", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x37\x6d"},
    {"238", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x38\x6d"},
    {"239", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x33\x39\x6d"},
    {"240", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x30\x6d"},
    {"241", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x31\x6d"},
    {"242", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x32\x6d"},
    {"243", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x33\x6d"},
    {"244", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x34\x6d"},
    {"245", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x35\x6d"},
    {"246", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x36\x6d"},
    {"247", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x37\x6d"},
    {"248", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x38\x6d"},
    {"249", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x34\x39\x6d"},
    {"250", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x30\x6d"},
    {"251", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x31\x6d"},
    {"252", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x32\x6d"},
    {"253", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x33\x6d"},
    {"254", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x34\x6d"},
    {"255", "\x1b\x5b\x33\x38\x3b\x35\x3b\x32\x35\x35\x6d"},
};

static char normal[] = "\x1b\x28\x42\x1b\x5b\x6d";

static int CompareColorName(const void* a, const void* b) {
  const Color* aa = a;
  const Color* bb = b;
  return strcmp(aa->name, bb->name);
}

static const char* FindEscape(const char* name) {
  const Color key = {.name = name};
  size_t count = COUNT(colors);
  Color* escape = lfind(&key, colors, &count, sizeof(Color), CompareColorName);
  if (!escape) {
    count = COUNT(extended_colors);
    escape =
        lfind(&key, extended_colors, &count, sizeof(Color), CompareColorName);
  }
  return escape->escape;
}

typedef struct Pattern {
  regex_t regex;
  Color color;
} Pattern;

typedef struct Patterns {
  size_t count;
  Pattern* patterns;
} Patterns;

static void FreePatterns(Patterns* p) {
  if (!p) {
    return;
  }
  for (size_t i = 0; i < p->count; i++) {
    regfree(&(p->patterns[i].regex));
  }
}

static void PrintRegexError(int error, regex_t* regex) {
  char message[512] = "";
  (void)regerror(error, regex, message, sizeof(message));
  MustPrintf(stderr, "bad RE: %s\n", message);
}

static void PrintColors(bool extended) {
  fputs("The available colors are:\n", stdout);
  for (size_t i = 0; i < COUNT(colors); i++) {
    MustPrintf(stdout, "\t· %s%s%s\n", colors[i].escape, colors[i].name,
               normal);
  }
  if (extended) {
    for (size_t i = 0; i < COUNT(extended_colors); i++) {
      MustPrintf(stdout, "\t· %s%s%s\n", extended_colors[i].escape,
                 extended_colors[i].name, normal);
    }
  }
}

typedef struct FindResult {
  size_t pattern;
  regmatch_t match;
} FindResult;

static FindResult FindFirstMatch(const char* input, Patterns patterns) {
  FindResult result;
  memset(&result, 0, sizeof(result));
  bool found = false;
  for (size_t i = 0; i < patterns.count; i++) {
    regmatch_t match;
    const int e = regexec(&(patterns.patterns[i].regex), input, 1, &match, 0);
    if (e) {
      if (e != REG_NOMATCH) {
        PrintRegexError(e, &(patterns.patterns[i].regex));
      }
      continue;
    } else if (!found || match.rm_so < result.match.rm_so) {
      result.match = match;
      result.pattern = i;
      found = true;
    }
  }
  return result;
}

static void Colorize(Patterns patterns, char delimiter) {
  AUTO(char*, line, NULL, FreeChar);
  size_t capacity = 0;
  while (true) {
    ssize_t length = getdelim(&line, &capacity, delimiter, stdin);
    if (length < 0) {
      return;
    }
    if (length && line[length - 1] == delimiter) {
      line[length - 1] = '\0';
      length--;
    }

    char* ln = line;
    while (length) {
      FindResult result = FindFirstMatch(ln, patterns);
      const regmatch_t* match = &result.match;
      if (match->rm_eo - match->rm_so) {
        const Pattern* p = &(patterns.patterns[result.pattern]);
        fwrite(ln, 1, (size_t)match->rm_so, stdout);
        fwrite(p->color.escape, 1, strlen(p->color.escape), stdout);
        fwrite(&ln[match->rm_so], 1, (size_t)(match->rm_eo - match->rm_so),
               stdout);
        fwrite(normal, 1, strlen(normal), stdout);
        length -= match->rm_eo;
        ln = &ln[match->rm_eo];
      } else {
        fwrite(ln, 1, (size_t)length, stdout);
        break;
      }
    }
    fputs("\n", stdout);
  }
}

static Patterns BuildPatterns(size_t count, char** arguments) {
  Pattern* patterns = calloc(count / 2, sizeof(Pattern));
  for (size_t i = 0; i < count; i += 2) {
    Pattern* pattern = &patterns[i / 2];
    const int e =
        regcomp(&(pattern->regex), arguments[i], REG_EXTENDED | REG_ICASE);
    if (e) {
      PrintRegexError(e, &(pattern->regex));
      PrintHelp(true, help);
    }
    pattern->color.name = arguments[i + 1];
    pattern->color.escape = FindEscape(pattern->color.name);
  }
  return (Patterns){.count = count / 2, .patterns = patterns};
}

int main(int count, char** arguments) {
  opterr = 0;
  char delimiter = '\n';
  while (true) {
    const int o = getopt(count, arguments, "0hx");
    if (o == -1) {
      break;
    }
    switch (o) {
      case '0':
        delimiter = '\0';
        break;
      case 'h':
      case 'x':
        fputs(help, stdout);
        fputs("\n", stdout);
        PrintColors(o == 'x');
        return 0;
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  if (!count || count % 2) {
    PrintHelp(true, help);
  }

  AUTO(Patterns, patterns, BuildPatterns((size_t)count, arguments),
       FreePatterns);
  Colorize(patterns, delimiter);
}
