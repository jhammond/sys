#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trace.h"

const char *program_usage = "[OPTION]... PATH MODE";

int main(int argc, char *argv[])
{
  const char *path;
  mode_t mode;

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

  if (argc - optind < 2)
    usage(stderr, EXIT_FAILURE);

  path = argv[optind];
  mode = strtol(argv[optind + 1], NULL, 0);

  if (chmod(path, mode) < 0)
    FATAL("cannot change mode of `%s' to %05o: %m\n", path, mode);

  return 0;
}
