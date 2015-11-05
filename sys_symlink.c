#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  const char *old_path, *new_path;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s OLDPATH NEWPATH\n",
	    program_invocation_short_name);
    exit(EXIT_FAILURE);
  }

  old_path = argv[1];
  new_path = argv[2];

  if (symlink(old_path, new_path) < 0) {
    fprintf(stderr, "cannot create symbolic link '%s' containing '%s': %m\n",
	    new_path, old_path);
    exit(EXIT_FAILURE);
  }

  return 0;
}
