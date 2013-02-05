#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "lustre/lustre_user.h"

#ifdef DEBUG
#define TRACE ERROR
#else
static inline void TRACE(char *fmt, ...) { }
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
	int fd = -1;

	struct lov_user_md_v3 *lum = NULL;
	size_t stripe_count = 1;
 	size_t lum_size = sizeof(*lum) +
		stripe_count * sizeof(lum->lmm_objects[0]);

	fd = open(path, O_RDWR|O_CREAT|O_LOV_DELAY_CREATE, 0666);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);

	lum = malloc(lum_size);
	if (lum == NULL)
		FATAL("cannot allocate stripe info: %m\n");
	memset(lum, 0, lum_size);

	lum->lmm_magic = LOV_USER_MAGIC_V3;
	lum->lmm_pattern = 0;
        lum->lmm_stripe_size = 0;
        lum->lmm_stripe_count = stripe_count;
	lum->lmm_stripe_offset = -1;

	int rc = ioctl(fd, LL_IOC_LOV_SETSTRIPE, lum);
	if (rc < 0)
		FATAL("cannot get stripe info for '%s': %m\n", path);

	printf("lmm_magic         %8x\n"
	       "lmm_pattern       %8x\n"
	       "lmm_object_id     %8llx\n"
	       "lmm_object_seq    %8llx\n"
	       "lmm_stripe_size   %8x\n"
	       "lmm_stripe_count  %8hx\n"
	       "lmm_stripe_offset %8hx\n",
	       lum->lmm_magic,
	       lum->lmm_pattern,
	       lum->lmm_object_id,
	       lum->lmm_object_seq,
	       lum->lmm_stripe_size,
	       lum->lmm_stripe_count,
	       lum->lmm_stripe_offset);

	printf("%12s %12s %8s %8s\n",
	       "object_id", "object_seq", "ost_gen", "ost_idx");

	int i;
	for (i = 0; i < lum->lmm_stripe_count; i++) {
		struct lov_user_ost_data_v1 *o;

		o = &lum->lmm_objects[i];
		printf("%12llx %12llx %8x %8x\n",
		       o->l_object_id,
		       o->l_object_seq,
		       o->l_ost_gen,
		       o->l_ost_idx);
	}

	close(fd);

	return 0;
}
