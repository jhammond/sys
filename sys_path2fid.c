#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <inttypes.h>

struct lu_fid {
  /**
   * FID sequence. Sequence is a unit of migration: all files (objects)
   * with FIDs from a given sequence are stored on the same server.
   * Lustre should support 2^64 objects, so even if each sequence
   * has only a single object we can still enumerate 2^64 objects.
   */
  uint64_t f_seq;
  /** FID number within sequence. */
  uint32_t f_oid;
  /**
   * FID version, used to distinguish different versions (in the sense
   * of snapshots, etc.) of the same file system object. Not currently
   * used.
   */
  uint32_t f_ver;
};

#define LL_IOC_PATH2FID _IOR ('f', 173, long )

static void usage(FILE *file, int status)
{
  fprintf(file, "Usage: %s [OPTION]... PATH [FLAGS]\n"
          "Open file and call LL_IOC_PATH2FID.\n"
          "FLAGS: acdDerstw\n"
          "\n"
          "Mandatory arguments to long options are mandatory for short options too.\n"
          "  -h, --help\n"
          "  -m, --mode=MODE\n"
          "  -p, --pause\n"
          "  -s, --sleep=SECONDS\n"
          ,
	  program_invocation_short_name);
  exit(status);
}

int main(int argc, char *argv[])
{
  const char *path;
  mode_t mode = 0666;
  int want_pause = 0;
  unsigned int delay = 0;
  int flags = 0, o_read = 0, o_write = 0;
  int fd;
  struct lu_fid fid;
  char *s;

  struct option opts[] = {
    { "help", 0, NULL, 'h' },
    { "mode", 1, NULL, 'm' },
    { "pause", 0, NULL, 'p' },
    { "sleep", 1, NULL, 's' },
    { NULL }
  };

  int c;
  while ((c = getopt_long(argc, argv, "hm:ps:", opts, 0)) != -1) {
    switch (c) {
    case 'h':
      usage(stdout, EXIT_SUCCESS);
      break;
    case 'm':
      mode = strtol(optarg, NULL, 0);
      break;
    case 'p':
      want_pause = 1;
      break;
    case 's':
      delay = strtoul(optarg, NULL, 0);
      break;
    case '?':
      fprintf(stderr, "Try `%s --help' for more information\n",
	      program_invocation_short_name);
      exit(EXIT_FAILURE);
    }
  }

  if (argc - optind < 1)
    usage(stderr, EXIT_FAILURE);

  path = argv[optind];

  if (argc - optind == 1) {
    o_read = 1;
    goto have_flags;
  }

  s = argv[optind + 1];
  for (; *s != 0; s++) {
    switch(*s) {
    case 'a':
      flags |= O_APPEND;
      break;
    case 'c':
      flags |= O_CREAT;
      break;
    case 'd':
      flags |= O_DIRECT;
      break;
    case 'D':
      flags |= O_DIRECTORY;
      break;
    case 'e':
      flags |= O_EXCL;
      break;
    case 'r':
      o_read = 1;
      break;
    case 's':
      flags |= O_SYNC;
      break;
    case 't':
      flags |= O_TRUNC;
      break;
    case 'w':
      o_write = 1;
      break;
    }
  }

  if (o_read && o_write)
    flags |= O_RDWR;

  if (o_read && !o_write)
    flags |= O_RDONLY;

  if (!o_read && o_write)
    flags |= O_WRONLY;

 have_flags:
  fd = open(path, flags, mode);
  if (fd < 0) {
    fprintf(stderr, "cannot open `%s' with flags %d, mode %04o: %m\n",
	    path, flags, mode);
    exit(EXIT_FAILURE);
  }

  if (ioctl(fd, LL_IOC_PATH2FID, &fid) < 0) {
    fprintf(stderr, "cannot get FID for `%s': %m\n", path);
    exit(EXIT_FAILURE);
  }

  printf("[0x%"PRIx64":0x%"PRIx32":0x%"PRIx32"]\n",
         fid.f_seq, fid.f_oid, fid.f_ver);

  if (want_pause)
    pause();

  if (delay > 0)
    sleep(delay);

  return 0;
}
