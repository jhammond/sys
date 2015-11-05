#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "trace.h"

const char *program_usage = "[OPTION]... PATH...";
int want_hex = 0;
int want_raw = 0;

int sys_stat(const char *path)
{
  struct stat st;

  if (stat(path, &st) < 0) {
    fprintf(stderr, "cannot stat '%s': %s\n", path, strerror(errno));
    return -1;
  }

  /*
   * mode_t shall be an integer type.
   * nlink_t, uid_t, gid_t, and id_t shall be integer types.
   * blkcnt_t and off_t shall be signed integer types.
   * fsblkcnt_t, fsfilcnt_t, and ino_t shall be defined as unsigned integer types.
   * size_t shall be an unsigned integer type.
   * blksize_t, pid_t, and ssize_t shall be signed integer types.
   * time_t and clock_t shall be integer or real-floating types.
   */

  if (want_raw) {
    write(STDOUT_FILENO, &st, sizeof(st)); /* XXX */
    return 0;
  }

#define STAT_X \
  X(lu, dev_t,     dev,     "ID of device containing file"), \
  X(lu, ino_t,     ino,     "inode number"), \
  X(u,  mode_t,    mode,    "protection"), \
  X(u,  nlink_t,   nlink,   "number of hard links"), \
  X(u,  uid_t,     uid,     "user ID of owner"), \
  X(u,  gid_t,     gid,     "group ID of owner"), \
  X(lu, dev_t,     rdev,    "device ID (if special file)"), \
  X(ld, off_t,     size,    "total size, in bytes"), \
  X(d,  blksize_t, blksize, "blocksize for file system I/O"), \
  X(ld, blkcnt_t,  blocks,  "number of 512B blocks allocated"), \
  X(ld, time_t,    atime,   "time of last access"), \
  X(ld, time_t,    mtime,   "time of last modification"), \
  X(ld, time_t,    ctime,   "time of last status change")

  typedef unsigned sys_u_t;
  typedef long unsigned sys_lu_t;
  typedef long sys_ld_t;
  typedef int sys_d_t;

  if (want_hex) {
#define X(pri,type,mem,desc) \
    printf("%-8s %16lx\n", #mem, (unsigned long) st.st_##mem)
    STAT_X;
#undef X
  } else {
#define X(pri,type,mem,desc) \
    printf("%-8s %16"#pri"\n", #mem, (sys_##pri##_t) st.st_##mem)
    STAT_X;
#undef X
  }

  return 0;
}

int main(int argc, char *argv[])
{
  struct option opts[] = {
    { "help", 0, NULL, 'h' },
    { "raw",  0, NULL, 'r' },
    { "hex",  0, NULL, 'x' },
    { NULL }
  };

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

  int i;
  for (i = optind; i < argc; i++)
    if (sys_stat(argv[i]) < 0)
      return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
