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
    { "raw",  0, NULL, 'r' },
    { "hex",  0, NULL, 'x' },
    { NULL }
  };
  int want_hex = 0;
  int want_raw = 0;

  int c;
  while ((c = getopt_long(argc, argv, "hrx", opts, 0)) != -1) {
    switch (c) {
    case 'h':
      usage(stdout, EXIT_SUCCESS);
      break;
    case 'r':
      want_raw = 1;
      break;
    case 'x':
      want_hex = 1;
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
    FATAL("cannot open `%s': %s\n", path, strerror(errno));

  while (1) {
    char buf[4096];
    int nr = syscall(SYS_getdents64, dfd, buf, sizeof(buf));

    if (nr < 0)
      FATAL("cannot read directory `%s': %s\n", path, strerror(errno));

    if (nr == 0)
      break;

    if (want_raw) {
      write(STDOUT_FILENO, buf, nr);
      continue;
    }

    /* 
       struct dirent64 {
         __ino64_t d_ino;
         __off64_t d_off;
         unsigned short int d_reclen;
         unsigned char d_type;
         char d_name[256];
       };
    */

    char *p = buf;
    while (p < buf + nr) {
      struct dirent64 *d = (struct dirent64 *) p;
      int d_type_c;

      switch (d->d_type) {
      default:
	d_type_c = '!';
	break;
      case DT_UNKNOWN:
	d_type_c = '?';
	break;
      case DT_FIFO:
	d_type_c = 'p';
	break;
      case DT_CHR:
	d_type_c = 'c';
	break;
      case DT_DIR:
	d_type_c = 'd';
	break;
      case DT_BLK:
	d_type_c = 'b';
	break;
      case DT_REG:
	d_type_c = 'r';
	break;
      case DT_LNK:
	d_type_c = 'l';
	break;
      case DT_SOCK:
	d_type_c = 's';
	break;
      case DT_WHT:
	d_type_c = 'w';
	break;
      }

      if (want_hex)
	printf("%16"PRIx64" %16"PRIx64" %4hx %2d-%c `%s'\n",
	       d->d_ino, (uint64_t) d->d_off, (unsigned short) d->d_reclen,
	       (int) d->d_type, d_type_c, d->d_name);
      else
	printf("%18"PRIu64" %20"PRId64" %4hu %2x-%c `%s'\n",
	       d->d_ino, (int64_t) d->d_off, d->d_reclen,
	       (unsigned) d->d_type, d_type_c, d->d_name);

      p += d->d_reclen;
    }
  }

  return 0;
}
