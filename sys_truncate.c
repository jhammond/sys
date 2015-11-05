#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  const char *path;
  long long len;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PATH [LENGTH]\n",
	    program_invocation_short_name);
    exit(EXIT_FAILURE);
  }

  path = argv[1];
  len = argc < 3 ? 0 : strtoll(argv[2], NULL, 0);

  if ((off_t) len != len)
    /* ... */;

  if (truncate(path, len) < 0) {
    fprintf(stderr, "cannot truncate '%s' to length %lld: %m\n",
	    path, len);
    exit(EXIT_FAILURE);
  }

  return 0;
}
