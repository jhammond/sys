#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  const char *path;
  struct stat st = { };

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PATH\n",
	    program_invocation_short_name);
    exit(EXIT_FAILURE);
  }

  path = argv[1];

  if (stat(path, &st) < 0) {
    fprintf(stderr, "cannot stat `%s': %s\n", path, strerror(errno));
    exit(EXIT_FAILURE);
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

#define X(pri,type,mem,desc) \
  printf("%-8s %16"#pri"\n", #mem, (sys_##pri##_t) st.st_##mem)
  STAT_X;
#undef X

  return 0;
}
