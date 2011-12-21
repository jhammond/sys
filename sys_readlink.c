#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trace.h"

const char *program_usage = "[OPTION]... PATH";

int main(int argc, char *argv[])
{
  const char *path;
  char buf[4096];

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
      bad_option();
      break;
    }
  }

  if (argc - optind < 1)
    usage(stderr, EXIT_FAILURE);

  path = argv[optind];

  ssize_t len = readlink(path, buf, sizeof(buf));
  if (len < 0)
    FATAL("cannot read link `%s': %m\n", path);

  printf("`%s' `%.*s'\n", path, (int) len, buf);

  return 0;
}
