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

#define ERROR(fmt,args...) \
    fprintf(stderr, "%s: "fmt, program_invocation_short_name, ##args)

#define FATAL(fmt,args...) \
  do { \
    ERROR(fmt, ##args);\
    exit(1);\
  } while (0)

int main(int argc, char *argv[])
{
  const char *path = argv[1];
  int fd = open(path, O_RDONLY);
  if (fd < 0)
    FATAL("cannot open '%s': %m\n", path);

  size_t stripe_count_max = 1024;
  struct lov_user_md_v1 *lum;
  size_t lum_size = sizeof(*lum) + stripe_count_max * sizeof(struct lov_user_ost_data_v1);
  lum = malloc(lum_size);
  if (lum == NULL)
    FATAL("cannot allocate stripe info: %m\n");

  memset(lum, 0, lum_size);
  lum->lmm_magic = LOV_USER_MAGIC_V1;
  lum->lmm_stripe_count = stripe_count_max;

  int rc = ioctl(fd, LL_IOC_LOV_GETSTRIPE, lum);
  if (rc < 0)
    FATAL("cannot get stripe info for '%s': %m\n", path);

  printf("lmm_magic         %8x\n"
         "lmm_pattern       %8x\n"
         "lmm_object_id     %8lx\n"
         "lmm_object_gr     %8lx\n"
         "lmm_stripe_size   %8x\n"
         "lmm_stripe_count  %8x\n"
         "lmm_stripe_offset %8x\n",
         lum->lmm_magic,
         lum->lmm_pattern,
         (unsigned long) lum->lmm_object_id,
         (unsigned long) lum->lmm_object_gr,
         lum->lmm_stripe_size,
         (unsigned) lum->lmm_stripe_count,
         (unsigned) lum->lmm_stripe_offset);

  printf("%12s %12s %8s %8s\n",
         "object_id", "object_gr", "ost_gen", "ost_idx");

  int i;
  for (i = 0; i < lum->lmm_stripe_count; i++) {
    struct lov_user_ost_data_v1 *o = &lum->lmm_objects[i];
    printf("%12lx %12lx %8x %8x\n",
           (unsigned long) o->l_object_id,
           (unsigned long) o->l_object_gr,
           (unsigned) o->l_ost_gen,
           (unsigned) o->l_ost_idx);
  }

  close(fd);

  return 0;
}
