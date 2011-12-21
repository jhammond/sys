#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  const char *path;
  struct stat buf;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PATH\n",
	    program_invocation_short_name);
    exit(EXIT_FAILURE);
  }

  path = argv[1];

  if (stat(path, &buf) < 0) {
    fprintf(stderr, "cannot stat `%s': %m\n", path);
    exit(EXIT_FAILURE);
  }

  return 0;
}
