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
#include "trace.h"

#define min_t(type, x, y)				\
	({						\
		type __min1 = (x);			\
		type __min2 = (y);			\
		__min1 < __min2 ? __min1: __min2;	\
	})

int main(int argc, char *argv[])
{
	const char *dir_path = argv[1];
	const char *name = argv[2];
	size_t stripe_count = atoi(argv[3]);

	int dir_fd = -1;
	struct lov_user_md_v3 *lum;
 	size_t lum_size = sizeof(*lum) +
		min_t(size_t, strlen(name),
		      stripe_count * sizeof(lum->lmm_objects[0]));
	int i, rc;

	dir_fd = open(dir_path, O_RDONLY);
	if (dir_fd < 0)
		FATAL("cannot open '%s': %m\n", dir_path);

	lum = malloc(lum_size);
	if (lum == NULL)
		FATAL("cannot allocate stripe info: %m\n");
	memset(lum, 0, sizeof(*lum));

	lum->lmm_magic = LOV_USER_MAGIC_V3;
	lum->lmm_stripe_count = stripe_count;

	strcpy((char *)lum->lmm_objects, name);

	rc = ioctl(dir_fd, LL_IOC_GETFILESTRIPE_V3, lum);
	if (rc < 0 && errno != ERANGE) {
		FATAL("cannot get stripe info for '%s' in '%s': %s\n",
		      name, dir_path, strerror(errno));
	}

	if (lum->lmm_magic != LOV_USER_MAGIC_V1 &&
	    lum->lmm_magic != LOV_USER_MAGIC_V3)
		FATAL("'%s' in '%s' has unexpected stripe md magic %x, expected %x\n",
		      name, dir_path, lum->lmm_magic, LOV_USER_MAGIC_V1);

	printf("lmm_magic         %12x\n"
	       "lmm_pattern       %12x\n"
	       "lmm_object_id     %12lx\n"
	       "lmm_object_seq    %12lx\n"
	       "lmm_stripe_size   %12x\n"
	       "lmm_stripe_count  %12x\n"
	       "lmm_stripe_offset %12x\n",
	       lum->lmm_magic,
	       lum->lmm_pattern,
	       (unsigned long) lum->lmm_object_id,
	       (unsigned long) lum->lmm_object_seq,
	       lum->lmm_stripe_size,
	       (unsigned) lum->lmm_stripe_count,
	       (unsigned) lum->lmm_stripe_offset);

	if (lum->lmm_magic == LOV_USER_MAGIC_V3)
		printf("lmm_pool_name     %16s\n",
		       lum->lmm_pool_name);

	printf("%12s %12s %8s %8s\n",
	       "object_id", "object_seq", "ost_gen", "ost_idx");

	struct lov_user_ost_data_v1 *objs;
	if (lum->lmm_magic == LOV_USER_MAGIC_V1)
		objs = &((struct lov_user_md_v1 *) lum)->lmm_objects[0];
	else
		objs = &((struct lov_user_md_v3 *) lum)->lmm_objects[0];

	if (lum->lmm_stripe_count > stripe_count)
		FATAL("returned stripe_count (%hu) exceeds allocated stripe count (%zu)\n",
		      lum->lmm_stripe_count, stripe_count);

	for (i = 0; i < lum->lmm_stripe_count; i++) {
		struct lov_user_ost_data_v1 *o = &objs[i];
		printf("%12lx %12lx %8x %8x\n",
		       (unsigned long) o->l_object_id,
		       (unsigned long) o->l_object_seq,
		       (unsigned) o->l_ost_gen,
		       (unsigned) o->l_ost_idx);
	}

	close(dir_fd);

	return 0;
}
