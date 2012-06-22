#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include "lustre/liblustreapi.h"
#include <errno.h>

#ifdef DEBUG
#define TRACE ERROR
#else
static inline void TRACE(char *fmt, ...) { }
#endif

#if 0
#undef LL_IOC_LOV_GETSTRIPE
#define LL_IOC_LOV_GETSTRIPE _IOW('f', 155, long)
#undef LOV_USER_MAGIC_V3
#define LOV_USER_MAGIC_V3 0x0bd30bd0
#endif

#ifndef MAX_OBD_NAME
#define MAX_OBD_NAME 128
#endif

#define OBD_IOC_GETNAME _IOWR('f', 131, long)

#define ERROR(fmt,args...) \
    fprintf(stderr, "%s: "fmt, program_invocation_short_name, ##args)

#define FATAL(fmt,args...) \
  do { \
    ERROR(fmt, ##args);\
    exit(1);\
  } while (0)

int get_ost_addr(char *buf, size_t size, const char *fs, void *sb, unsigned idx)
{
  /* See server_make_name(). */
  /* /proc/fs/lustre/osc/scratch-OST00b0-osc-ffff81042f2c8800/ost_conn_uuid */
  char path[4096];
  int rc = -1, fd = -1;

  snprintf(path, sizeof(path),
           "/proc/fs/lustre/osc/%.8s-OST%04x-osc-%016llx/ost_conn_uuid",
           fs, idx, (unsigned long long) sb);

  TRACE("path `%s'\n", path);

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    ERROR("cannot open `%s': %m\n", path);
    goto out;
  }

  ssize_t nr = read(fd, buf, size);
  if (nr < 0) {
    ERROR("error reading from `%s': %m\n", path);
    goto out;
  } else if (nr >= size) {
    ERROR("ost_conn_uuid `%.*s' too long\n", (int) size, buf);
    goto out;
  }

  char *lf = memchr(buf, '\n', nr);
  if (lf != NULL)
    *lf = 0;
  else
    buf[nr] = 0;

  rc = 0;
 out:
  if (fd >= 0)
    close(fd);

  return rc;
}

int main(int argc, char *argv[])
{
  const char *path = argv[1];
  int fd = open(path, O_RDONLY);
  if (fd < 0)
    FATAL("cannot open `%s': %m\n", path);

  struct lov_user_md_v1 *lum;
  size_t stripe_count_max = 1024;
  size_t lum_size = sizeof(*lum) + stripe_count_max * sizeof(struct lov_user_ost_data_v1);

  lum = malloc(lum_size);
  if (lum == NULL)
    FATAL("cannot allocate stripe info: %m\n");

  memset(lum, 0, lum_size);
  lum->lmm_magic = LOV_USER_MAGIC_V1;
  lum->lmm_stripe_count = stripe_count_max;

  if (ioctl(fd, LL_IOC_LOV_GETSTRIPE, lum) < 0)
    FATAL("cannot get stripe info for `%s': %m\n", path);

  char obd[MAX_OBD_NAME];
  char fs[MAX_OBD_NAME];
  void *sb = NULL;

  if (ioctl(fd, OBD_IOC_GETNAME, obd) < 0) {
    ERROR("cannot get obd_name for `%s': %m\n", path);
    strcpy(obd, "-");
    strcpy(fs, "-");
    goto no_obd;
  }

  /* Example obd_name lustre-clilov-ffff88002738bc00. */
  snprintf(fs, sizeof(fs), "%.*s", (int) strlen(obd) - 24, obd);
  sscanf(obd + strlen(obd) - 16, "%p", &sb);

  TRACE("obd `%s', fs `%s', sb %p\n", obd, fs, sb);

 no_obd:
  printf("file             `%s'\n"
         "obd_name         `%s'\n"
         "lmm_magic         %8x\n"
         "lmm_pattern       %8x\n"
         "lmm_object_id     %8lx\n"
         "lmm_stripe_size   %8x\n"
         "lmm_stripe_count  %8x\n"
         "lmm_stripe_offset %8x\n",
         path,
         obd,
         lum->lmm_magic,
         lum->lmm_pattern,
         (unsigned long) lum->lmm_object_id,
         lum->lmm_stripe_size,
         (unsigned) lum->lmm_stripe_count,
         (unsigned) lum->lmm_stripe_offset);

  printf("%12s %12s %8s %8s %-20s\n",
         "object_id", "object_gr", "ost_gen", "ost_idx", "ost_conn");

  int i;
  for (i = 0; i < lum->lmm_stripe_count; i++) {
    struct lov_user_ost_data_v1 *o = &lum->lmm_objects[i];
    char addr[40];

    if (sb == NULL || get_ost_addr(addr, sizeof(addr), fs, sb, o->l_ost_idx) < 0)
      strcpy(addr, "-");

    printf("%12lx %8x %8x %-20s\n",
           (unsigned long) o->l_object_id,
           (unsigned) o->l_ost_gen,
           (unsigned) o->l_ost_idx,
           addr);
  }

  close(fd);

  return 0;
}
