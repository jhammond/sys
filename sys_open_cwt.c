#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  const char *path = argv[1];
  mode_t mode = 0666;

  if (open(path, O_CREAT|O_WRONLY|O_TRUNC, mode) < 0)
    fprintf(stderr, "cannot open `%s' with mode %04o: %m\n",
	    path, mode);

  return 0;
}
