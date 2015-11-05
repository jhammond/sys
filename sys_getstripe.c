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
#include "sys_lustre.h"
#include "trace.h"

int main(int argc, char *argv[])
{
	const char *path = argv[1];
	int fd = -1;

	struct lov_user_md_v1 *lum;
	size_t stripe_count = LOV_MAX_STRIPE_COUNT;
 	size_t lum_size = sizeof(*lum) +
		stripe_count * sizeof(lum->lmm_objects[0]);

	fd = open(path, O_RDONLY);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);

	lum = malloc(lum_size);
	if (lum == NULL)
		FATAL("cannot allocate stripe info: %m\n");
	memset(lum, 0, lum_size);

	lum->lmm_magic = LOV_USER_MAGIC_V1;
	lum->lmm_stripe_count = stripe_count;

	memset(&lum->lmm_objects[0], 0xff,
	       stripe_count * sizeof(lum->lmm_objects[0]));

	int rc = ioctl(fd, LL_IOC_LOV_GETSTRIPE, lum);
	if (rc < 0)
		FATAL("cannot get stripe info for '%s': %m\n", path);

	if (lum->lmm_magic != LOV_USER_MAGIC_V1 &&
	    lum->lmm_magic != LOV_USER_MAGIC_V3)
		FATAL("'%s' unexpected stripe md magic %x, expected %x\n",
		      path, lum->lmm_magic, LOV_USER_MAGIC_V1);

	fprint_lov_user_md(stdout, lum, sys_lov_user_md_size(lum));

#if 0
	printf("lmm_magic         %12x\n"
	       "lmm_pattern       %12x\n"
	       "lmm_released      %12d\n"
	       "lmm_oi_id         %12lx\n"
	       "lmm_oi_seq        %12lx\n"
	       "lmm_stripe_size   %12x\n"
	       "lmm_stripe_count  %12x\n"
	       "lmm_layout_gen    %12x\n",
	       lum->lmm_magic,
	       lov_pattern(lum->lmm_pattern),
	       (lum->lmm_pattern & LOV_PATTERN_F_RELEASED) != 0,
	       (unsigned long) lmm_oi_id(&lum->lmm_oi),
	       (unsigned long) lmm_oi_seq(&lum->lmm_oi),
	       lum->lmm_stripe_size,
	       (unsigned) lum->lmm_stripe_count,
	       (unsigned) lum->lmm_layout_gen);

	printf("%12s %12s %8s %8s\n",
	       "object_id", "object_seq", "ost_gen", "ost_idx");

	struct lov_user_ost_data_v1 *objs;
	if (lum->lmm_magic == LOV_USER_MAGIC_V1)
		objs = &((struct lov_user_md_v1 *) lum)->lmm_objects[0];
	else
		objs = &((struct lov_user_md_v3 *) lum)->lmm_objects[0];

	int i;
	for (i = 0; i < lum->lmm_stripe_count; i++) {
		struct lov_user_ost_data_v1 *o = &objs[i];
		printf("%12lx %12lx %8x %8x\n",
		       (unsigned long) lmm_oi_id(&o->l_ost_oi),
		       (unsigned long) lmm_oi_seq(&o->l_ost_oi),
		       (unsigned) o->l_ost_gen,
		       (unsigned) o->l_ost_idx);
	}
#endif

	close(fd);

	return 0;
}
