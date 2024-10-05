// Copyright 2024 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <assert.h>
#include <errno.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

// clang-format off
static char help[] =
"expand [-0] [-j job-count] [-n argument-count] program [arguments...]\n"
"\n"
"To explicitly place the argument(s) in the argument list, use %a.\n";
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

static int CompareJobs(const void* job1, const void* job2) {
  const Job* j1 = job1;
  const Job* j2 = job2;
  if (j1->pid < j2->pid) {
    return -1;
  } else if (j1->pid > j2->pid) {
    return 1;
  }
  return 0;
}

static Job* FindJob(pid_t pid) {
  const Job j = {.pid = pid};
  size_t count = max_job_count;
  return lfind(&j, jobs, &count, sizeof(*jobs), CompareJobs);
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
          fprintf(stderr,
                  "record '%s' too long to fit into command size limit (%zu)\n",
                  line, conservative_limit);
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

static enum {
  Continue,
  Complete,
} FillCommandLine(Job* job, int count, char** arguments) {
  bool have_read = false;
  const size_t count_allocated = (size_t)count + max_argument_count + 1;
  job->arguments = calloc(count_allocated, sizeof(char*));
  size_t a = 0;

  for (int i = 0; i < count; i++, arguments++) {
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

static void noreturn RunJobs(int count, char** arguments) {
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
        perror("vfork");
        continue;
      } else if (child) {
        jobs[i].pid = child;
      } else {
        if (execvp(jobs[i].arguments[0], jobs[i].arguments)) {
          perror(jobs[i].arguments[0]);
          _exit(errno);
        }
      }
    }

    int status;
    const pid_t child = waitpid(-1, &status, 0);
    if (child == -1) {
      if (errno != ECHILD) {
        perror("waitpid");
      }
      ReleaseJobs();
      exit(errno != ECHILD);
    }
    Job* j = FindJob(child);
    assert(j);
    ReleaseJob(j);
  }
}

int main(int count, char** arguments) {
  while (true) {
    const int o = getopt(count, arguments, "0hj:n:");
    if (o == -1) {
      break;
    }
    switch (o) {
      case '0':
        delimiter = '\0';
        break;
      case 'h':
        PrintHelp(false, help);
      case 'n':
        max_argument_count = strtoul(optarg, NULL, 0);
        break;
      case 'p':
        max_job_count = strtoul(optarg, NULL, 0);
        break;
      default:
        PrintHelp(true, help);
    }
  }
  count -= optind;
  arguments += optind;

  long n = sysconf(_SC_ARG_MAX);
  if (n < 0) {
    perror("could not determine maximum command size");
    exit(1);
  }
  max_command_size = (size_t)n;
  max_argument_count = max_argument_count ? max_argument_count : 100;
  if (max_job_count == 0) {
    n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 0) {
      perror("could not determine processor count");
      exit(1);
    }
    max_job_count = (size_t)n;
  }

  jobs = calloc(max_job_count, sizeof(Job));
  RunJobs(count, arguments);
}
