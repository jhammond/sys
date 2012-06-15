#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <dirent.h> /* DT_* constants */
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include "trace.h"

const char *program_usage = "[OPTION]... PATH";

int main(int argc, char *argv[])
{
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

  const char *path = argv[optind];

  int dfd = open(path, O_RDONLY|O_DIRECTORY);
  if (dfd < 0)
    FATAL("cannot open `%s': %m\n", path);

  while (1) {
    char buf[4096];
    int nr = syscall(SYS_getdents64, dfd, buf, sizeof(buf));

    if (nr < 0)
      FATAL("cannot read directory `%s': %m\n", path);

    if (nr == 0)
      break;

    char *p = buf;
    while (p < buf + nr) {
      struct dirent64 *d = (struct dirent64 *) p;
      char d_type;

      switch (*(p + d->d_reclen - 1)) {
      default:
      case DT_UNKNOWN:
	d_type = '?';
	break;
      case DT_FIFO:
	d_type = 'p';
	break;
      case DT_CHR:
	d_type = 'c';
	break;
      case DT_DIR:
	d_type = 'd';
	break;
      case DT_BLK:
	d_type = 'b';
	break;
      case DT_REG:
	d_type = '-';
	break;
      case DT_LNK:
	d_type = 'l';
	break;
      case DT_SOCK:
	d_type = 's';
	break;
      case DT_WHT:
	d_type = 'w';
	break;
      }

      printf("%18"PRIu64" %20"PRId64" %4hu %c `%s'\n",
	     d->d_ino, (int64_t) d->d_off, d->d_reclen, d_type, d->d_name);

      p += d->d_reclen;
    }
  }

  return 0;
}
