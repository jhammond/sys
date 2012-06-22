CFLAGS = -Wall -Werror -D_GNU_SOURCE

SYS_CMDS = \
  sys_access \
  sys_chdir \
  sys_chmod \
  sys_chown \
  sys_creat \
  sys_getdents \
  sys_lea \
  sys_link \
  sys_listxattr \
  sys_lma \
  sys_lstat \
  sys_mkdir \
  sys_mkdirat \
  sys_mknod \
  sys_open \
  sys_path2fid \
  sys_readlink \
  sys_rename \
  sys_rmdir \
  sys_stat \
  sys_symlink \
  sys_truncate \
  sys_unlink

all: $(SYS_CMDS)

.PHONY: clean
clean:
	rm -f $(SYS_CMDS)
