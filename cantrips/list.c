// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <err.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static const char help[] =
"d [file...]\n"
"d -h\n"
"\n"
"-A      print the status of hidden files, too\n"
"-h      print this help message\n";
// clang-format on

static void PrintStatus(const char* pathname) {
  struct stat status;
  if (lstat(pathname, &status)) {
    warn("%s", pathname);
    return;
  }

  const struct tm* t = gmtime(&status.st_mtime);

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

  mode_t m = status.st_mode;
  char mode[11];
  char type = '-';
  if (m & S_IFDIR) {
    type = 'd';
  } else if (m & S_IFREG) {
    type = '-';
  } else if (m & S_IFLNK) {
    type = 'l';
  }
  MustFormat(
      mode, sizeof(mode), "%c%c%c%c%c%c%c%c%c%c", type, m & S_IRUSR ? 'r' : '-',
      m & S_IWUSR ? 'w' : '-', m & S_IXUSR ? 'x' : '-', m & S_IRGRP ? 'r' : '-',
      m & S_IWGRP ? 'w' : '-', m & S_IXGRP ? 'x' : '-', m & S_IROTH ? 'r' : '-',
      m & S_IWOTH ? 'w' : '-', m & S_IXOTH ? 'x' : '-');
  mode[3] = m & S_ISUID ? 's' : mode[3];
  mode[6] = m & S_ISGID ? 's' : mode[6];
  mode[9] = m & S_ISVTX ? 's' : mode[9];

  printf("%04d-%02d-%02d %02d:%02d  %12lld  %-12s  %-12s  %-10s  %s\n",
         t->tm_year + 1900, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min,
         (long long)status.st_size, u ? u->pw_name : u_buffer,
         g ? g->gr_name : g_buffer, mode, pathname);
}

int main(int count, char** arguments) {
  bool show_hidden = false;
  opterr = 0;
  while (true) {
    const int o = getopt(count, arguments, "Ah");
    if (o == -1) {
      break;
    }
    switch (o) {
      case 'A':
        show_hidden = true;
        break;
      case 'h':
        PrintHelp(false, help);
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  fprintf(stderr, "%-16s  %12s  %-12s  %-12s  %-10s  %s\n", "Modified", "Size",
          "User", "Group", "Mode", "Name");
  if (count == 0) {
    AUTO(DIR*, cwd, opendir("."), CloseDir);
    while (true) {
      const struct dirent* e = readdir(cwd);
      if (!e) {
        break;
      }
      if (show_hidden || e->d_name[0] != '.') {
        PrintStatus(e->d_name);
      }
    }
  }
  for (int i = 0; i < count; i++) {
    PrintStatus(arguments[i]);
  }
}
