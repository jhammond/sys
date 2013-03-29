CPPFLAGS += -D_GNU_SOURCE \
            -I/root/lustre-release/lustre/include \
            -I/root/lustre-release/libcfs/include

CFLAGS += -Wall -Werror

SYS_CMDS = \
  sys_2523 \
  sys_2808 \
  sys_2808_simple \
  sys_access \
  sys_chdir \
  sys_chmod \
  sys_chown \
  sys_copy_lov \
  sys_creat \
  sys_delay_setxattr \
  sys_delay_truncate \
  sys_getdents \
  sys_getdents_race \
  sys_getstripe \
  sys_getstripe_ea \
  sys_getstripe_path \
  sys_hsm_request \
  sys_lea \
  sys_link \
  sys_listxattr \
  sys_lma \
  sys_lstat \
  sys_mkdir \
  sys_mkdirat \
  sys_mknod \
  sys_o_delay_race \
  sys_o_delay_truncate \
  sys_o_delay_ftruncate \
  sys_o_delay_open_ftruncate \
  sys_o_delay_open_o_trunc \
  sys_open \
  sys_open_delay_create \
  sys_path2fid \
  sys_readlink \
  sys_rename \
  sys_rename_race \
  sys_rmdir \
  sys_setstripe \
  sys_stat \
  sys_swap_layouts \
  sys_swap_layouts_race \
  sys_symlink \
  sys_truncate \
  sys_unlink

all: $(SYS_CMDS)

.PHONY: clean
clean:
	rm -f $(SYS_CMDS)
