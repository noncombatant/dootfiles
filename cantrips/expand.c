// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cli.h"
#include "utils.h"

// clang-format off
static char description[] =
"turn records from the standard input into arguments to a command\n"
"\n"
"    expand [options...] program -- [options and arguments...]\n"
"\n"
"To explicitly place the argument(s) in the argument list, use %a.";

static Option options[] = {
  {
    .flag = '0',
    .description = "delimit input records with NUL instead of newline",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'a',
    .description = "pass each run of `program` at most this many arguments",
    .value = { .type = OptionTypeSize, .z = 100 }
  },
  {
    .flag = 'h',
    .description = "print help message",
    .value = { .type = OptionTypeBool }
  },
  {
    .flag = 'j',
    .description = "number of concurrent processes to handle the input",
    .value = { .type = OptionTypeSize }
  },
};

static CLI cli = {
  .name = "expand",
  .description = description,
  .options = {.count = COUNT(options), .values = options},
};
// clang-format on

typedef struct Job {
  pid_t pid;
  char** arguments;
} Job;

static size_t max_command_size;
static size_t max_argument_count;
static size_t max_job_count;
static char delimiter = '\n';
static Job* jobs;

static Job* FindJob(pid_t pid) {
  for (size_t i = 0; i < max_job_count; i++) {
    Job* j = &jobs[i];
    if (j->pid == pid) {
      return j;
    }
  }
  return NULL;
}

static void ReleaseJob(Job* j) {
  if (j->arguments) {
    char** a = j->arguments;
    while (*a) {
      char** next = a + 1;
      free(*a);
      a = next;
    }
    free(j->arguments);
  }
  memset(j, 0, sizeof(*j));
}

static void ReleaseJobs() {
  for (size_t i = 0; i < max_job_count; i++) {
    ReleaseJob(&jobs[i]);
  }
  free(jobs);
}

static size_t ReadArguments(char** arguments) {
  static char* line = NULL;
  static size_t capacity = 0;
  static size_t length = 0;

  char** a = arguments;
  size_t count = 0;
  size_t bytes_used = 0;
  const size_t conservative_limit = max_command_size / 2;

  while (true) {
    if (line && length) {
      if (count < max_argument_count &&
          conservative_limit - bytes_used > length) {
        *a = strndup(line, length);
        length = 0;
        a++;
        count++;
        bytes_used += length;
      } else {
        if (count == 0) {
          MustPrintf(
              stderr,
              "record '%s' too long to fit into command size limit (%zu)\n",
              line, conservative_limit);
          // TODO BUG?: Do we need to reset `bytes_used` here, to avoid infinite
          // loop?
        } else {
          return count;
        }
      }
    }

    const ssize_t n = getdelim(&line, &capacity, delimiter, stdin);
    if (n < 0) {
      return count;
    }
    length = (size_t)n;
    if (length && line[length - 1] == delimiter) {
      line[length - 1] = '\0';
      length--;
    }
  }
}

typedef enum FillStatus {
  Continue,
  Complete,
} FillStatus;

static FillStatus FillCommandLine(Job* job, size_t count, char** arguments) {
  bool have_read = false;
  const size_t count_allocated = (size_t)count + max_argument_count + 1;
  job->arguments = calloc(count_allocated, sizeof(char*));
  size_t a = 0;

  for (size_t i = 0; i < count; i++, arguments++) {
    if (StringEquals("%a", *arguments)) {
      const size_t r = ReadArguments(&job->arguments[a]);
      if (r == 0) {
        return Complete;
      }
      a += r;
      have_read = true;
    } else {
      job->arguments[a] = strdup(*arguments);
      a++;
    }
  }

  if (!have_read) {
    const size_t r = ReadArguments(&job->arguments[a]);
    if (r == 0) {
      return Complete;
    }
    a += r;
  }
  assert(a < count_allocated);
  return Continue;
}

static void noreturn RunJobs(size_t count, char** arguments) {
  while (true) {
    for (size_t i = 0; i < max_job_count; i++) {
      if (jobs[i].pid) {
        continue;
      }
      if (FillCommandLine(&jobs[i], count, arguments) == Complete) {
        break;
      }

      const pid_t child = vfork();
      if (child == -1) {
        Warn(errno, "vfork");
        continue;
      } else if (child) {
        jobs[i].pid = child;
      } else {
        if (execvp(jobs[i].arguments[0], jobs[i].arguments)) {
          Die(errno, "%s", jobs[i].arguments[0]);
        }
      }
    }

    int status;
    const pid_t child = waitpid(-1, &status, WUNTRACED);
    if (child == -1) {
      if (errno != ECHILD) {
        Warn(errno, "waitpid");
      }
      ReleaseJobs();
      exit(errno != ECHILD);
    }
    if (WIFSIGNALED(status)) {
      // If a child was signaled, terminate all of them and exit. The most
      // likely reasons are e.g. SIGINT or SIGPIPE, which are normal.
      if (kill(0, WTERMSIG(status))) {
        Die(errno, "kill");
      }
      exit(0);
    }
    Job* j = FindJob(child);
    assert(j);
    ReleaseJob(j);
  }
}

int main(int count, char** arguments) {
  long n = sysconf(_SC_NPROCESSORS_ONLN);
  if (n < 0) {
    Die(errno, "could not determine processor count");
  }
  FindOptionValue(cli.options, 'j')->z = (size_t)n;

  Arguments as = ParseCLI(&cli, count, arguments);
  if (FindOptionValue(cli.options, 'h')->b) {
    PrintHelpAndExit(&cli, false, true);
  } else if (as.count == 0) {
    PrintHelpAndExit(&cli, true, true);
  }

  n = sysconf(_SC_ARG_MAX);
  if (n < 0) {
    Die(errno, "could not determine maximum command size");
  }
  max_command_size = (size_t)n;
  if (FindOptionValue(cli.options, '0')->b) {
    delimiter = '\0';
  }
  max_argument_count = FindOptionValue(cli.options, 'a')->z;
  max_job_count = FindOptionValue(cli.options, 'j')->z;
  jobs = calloc(max_job_count, sizeof(Job));
  RunJobs(as.count, as.values);
}
