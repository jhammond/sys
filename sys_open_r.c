#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  const char *path = argv[1];

  if (open(path, O_RDONLY) < 0)
    fprintf(stderr, "cannot open `%s': %m\n", path);

  return 0;
}
