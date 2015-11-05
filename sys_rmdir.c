#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  const char *path;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PATH\n",
	    program_invocation_short_name);
    exit(EXIT_FAILURE);
  }

  path = argv[1];

  if (rmdir(path) < 0) {
    fprintf(stderr, "cannot delete directory '%s': %m\n", path);
    exit(EXIT_FAILURE);
  }

  return 0;
}
