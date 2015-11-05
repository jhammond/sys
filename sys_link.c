#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trace.h"

const char *program_usage = "[OPTION]... OLDPATH NEWPATH";

int main(int argc, char *argv[])
{
  const char *old_path, *new_path;

  if (argc < 3)
    usage(stderr, EXIT_FAILURE);

  old_path = argv[1];
  new_path = argv[2];

  if (link(old_path, new_path) < 0)
    FATAL("cannot link '%s' to '%s': %m\n", new_path, old_path);

  return 0;
}
