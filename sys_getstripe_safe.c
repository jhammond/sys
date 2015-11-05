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

int main(int argc, char *argv[])
{
	const char *path = argv[1];
	int fd = -1;

	struct lov_user_md_v1 *lum;
	size_t stripe_count = 1;
 	size_t lum_size = sizeof(*lum) +
		stripe_count * sizeof(lum->lmm_objects[0]);

	int guard_char = 'G';
	size_t guard_size = 64;
	size_t buf_size = lum_size + 2 * guard_size;
	char *buf;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);

	buf = malloc(buf_size);
	if (buf == NULL)
		FATAL("cannot allocate stripe info: %m\n");
	memset(buf, guard_char, buf_size);

	lum = (struct lov_user_md_v1 *)(buf + guard_size);
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
		       (unsigned long) o->l_object_id,
		       (unsigned long) o->l_object_seq,
		       (unsigned) o->l_ost_gen,
		       (unsigned) o->l_ost_idx);
	}

	for (i = 0; i < guard_size; i++) {
		if (buf[i] != guard_char) {
			ERROR("left guard corrupted\n");
			break;
		}
	}

	for (i = 0; i < guard_size; i++) {
		if (buf[i + guard_size + lum_size] != guard_char) {
			ERROR("left guard corrupted\n");
			break;
		}
	}

	close(fd);

	return 0;
}
