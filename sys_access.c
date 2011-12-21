#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include "trace.h"

const char *program_usage = "[OPTION]... PATH [MODE]";

int main(int argc, char *argv[])
{
  const char *path, *mode_str;
  int mode = 0;

  struct option opts[] = {
    { "help", 0, NULL, 'h' },
    { NULL }
  };

  int c;
  while ((c = getopt_long(argc, argv, "h", opts, 0)) != -1) {
    switch (c) {
    case 'h':
      usage(stdout, EXIT_SUCCESS);
      break;
    case '?':
      fprintf(stderr, "Try `%s --help' for more information\n",
	      program_invocation_short_name);
      exit(EXIT_FAILURE);
    }
  }

  if (argc - optind < 1)
    usage(stderr, EXIT_FAILURE);

  path = argv[optind];

  if (argc - optind < 2) {
    mode_str = "f";
    mode = F_OK;
    goto have_mode;
  }

  mode_str = argv[optind + 1];

  const char *p;
  for (p = mode_str; *p != 0; p++) {
    switch (*p) {
    case 'f':
      mode |= F_OK;
      break;
    case 'r':
      mode |= R_OK;
      break;
    case 'w':
      mode |= W_OK;
      break;
    case 'x':
      mode |= X_OK;
      break;
    }
  }

 have_mode:
  if (access(path, mode) < 0)
    FATAL("cannot access `%s' with mode `%s': %m\n", path, mode_str);

  return 0;
}
