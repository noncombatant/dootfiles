// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] = "Print names and properties of files.";

static Option options[] = {
  {
    .flag = 'A',
    .description = "print the status of hidden files, too",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'g',
    .description = "print times in GMT (default: local)",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'm',
    .description = "shuffle in memory (uses more memory but the shuffle is faster)",
    .value = { .type = OptionTypeBool }
  },
};

static CLI cli = {
  .name = "list",
  .description = description,
  .options = {.count = COUNT(options), .options = options},
};
// clang-format on

typedef struct tm* Time2Tm(const time_t* clock);
static Time2Tm* time2tm = localtime;

static void PrintStatus(const char* pathname) {
  struct stat status;
  if (lstat(pathname, &status)) {
    Warn(errno, "%s", pathname);
    return;
  }

  const struct tm* t = time2tm(&status.st_mtime);

  const struct passwd* u = getpwuid(status.st_uid);
  char u_buffer[13] = {0};
  if (!u) {
    MustFormat(u_buffer, sizeof(u_buffer), "%u", (unsigned)status.st_uid);
  }

  const struct group* g = getgrgid(status.st_gid);
  char g_buffer[13] = {0};
  if (!g) {
    MustFormat(g_buffer, sizeof(g_buffer), "%u", (unsigned)status.st_gid);
  }

  const mode_t m = status.st_mode;
  char type = '-';
  const char arrow[] = " → ";
  char target[sizeof(arrow) + PATH_MAX + 1] = {0};
  if (S_ISDIR(m)) {
    type = 'd';
  } else if (S_ISLNK(m)) {
    type = 'l';
    strncpy(target, arrow, strlen(arrow));
    const ssize_t r = readlink(pathname, target + strlen(arrow),
                               sizeof(target) - strlen(arrow));
    if (r == -1) {
      Warn(errno, "readlink(%s)", pathname);
      target[0] = '\0';
    }
  } else if (S_ISREG(m)) {
    type = '-';
  }

  char mode[11];
  MustFormat(
      mode, sizeof(mode), "%c%c%c%c%c%c%c%c%c%c", type, m & S_IRUSR ? 'r' : '-',
      m & S_IWUSR ? 'w' : '-', m & S_IXUSR ? 'x' : '-', m & S_IRGRP ? 'r' : '-',
      m & S_IWGRP ? 'w' : '-', m & S_IXGRP ? 'x' : '-', m & S_IROTH ? 'r' : '-',
      m & S_IWOTH ? 'w' : '-', m & S_IXOTH ? 'x' : '-');
  mode[3] = m & S_ISUID ? 's' : mode[3];
  mode[6] = m & S_ISGID ? 's' : mode[6];
  mode[9] = m & S_ISVTX ? 's' : mode[9];

  static bool printed_header = false;
  if (!printed_header) {
    MustPrintf(stdout, "%-16s  %12s  %-12s  %-12s  %-10s  %s\n", "Modified",
               "Size", "User", "Group", "Mode", "Name");
    printed_header = true;
  }
  MustPrintf(stdout,
             "%04d-%02d-%02d %02d:%02d  %12lld  %-12s  %-12s  %-10s  %s%s\n",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour,
             t->tm_min, (long long)status.st_size, u ? u->pw_name : u_buffer,
             g ? g->gr_name : g_buffer, mode, pathname,
             target[0] == '\0' ? "" : target);
}

int main(int count, char** arguments) {
  Arguments as = ParseCLI(&cli, count, arguments);
  if (FindOptionValue(&cli.options, 'h')->b) {
    PrintHelpAndExit(&cli, false, true);
  }
  if (as.count == 0) {
    AUTO(DIR*, cwd, opendir("."), CloseDir);
    while (true) {
      const struct dirent* e = readdir(cwd);
      if (!e) {
        break;
      }
      if (FindOptionValue(&cli.options, 'A')->b || e->d_name[0] != '.') {
        PrintStatus(e->d_name);
      }
    }
  }
  for (size_t i = 0; i < as.count; i++) {
    PrintStatus(as.arguments[i]);
  }
}
