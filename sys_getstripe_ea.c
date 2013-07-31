#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/xattr.h>
#include <errno.h>
#include "sys_lustre.h"
#include "trace.h"

int main(int argc, char *argv[])
{
	const char *path = argv[1];
	/* int fd = -1; */

        const char *lov_xattr_name = "trusted.lov";
        void *lov_xattr_buf;
	size_t lov_xattr_buf_size;
	ssize_t lov_xattr_size;
	const struct lov_user_md *lum;

 	lov_xattr_buf_size = XATTR_SIZE_MAX;

	lov_xattr_buf = malloc(lov_xattr_buf_size);
	if (lov_xattr_buf == NULL)
		FATAL("cannot allocate stripe info: %m\n");

	memset(lov_xattr_buf, 0xff, lov_xattr_buf_size);

#if 0
	fd = open(path, O_RDONLY);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);
#endif

	lov_xattr_size = getxattr(path, lov_xattr_name, lov_xattr_buf,
				  lov_xattr_buf_size);
        if (lov_xattr_size < 0)
                FATAL("cannot get xattr '%s' of '%s': %s\n",
                      lov_xattr_name, path, strerror(errno));

	ERROR("lov_xattr_size %zd\n", lov_xattr_size);

	if (lov_xattr_size < sizeof(*lum))
		FATAL("lov_xattr_size %zd too small for stripe md\n",
		      lov_xattr_size);

	lum = lov_xattr_buf;
	if (lum->lmm_magic != LOV_USER_MAGIC_V1 &&
	    lum->lmm_magic != LOV_USER_MAGIC_V3)
		FATAL("'%s' unexpected stripe md magic %x, expected %x\n",
		      path, lum->lmm_magic, LOV_USER_MAGIC_V1);

	fprint_lov_user_md(stdout, lum, lov_xattr_size);
#if 0

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

	const struct lov_user_ost_data_v1 *objs;
	if (lum->lmm_magic == LOV_USER_MAGIC_V1)
		objs = &((const struct lov_user_md_v1 *) lum)->lmm_objects[0];
	else
		objs = &((const struct lov_user_md_v3 *) lum)->lmm_objects[0];

	size_t lum_size = ((const char *) &objs[lum->lmm_stripe_count]) -
		((const char *) lum);

	if (lum_size != lov_xattr_size)
		ERROR("size (%zu) of stripe md with stripe count %u "
		      "is not equal to\n"
		      "size (%zu) of xattr '%s' on '%s'\n",
		      lum_size, (unsigned) lum->lmm_stripe_count,
		      lov_xattr_size, lov_xattr_name, path);

	printf("%12s %12s %8s %8s\n",
	       "object_id", "object_seq", "ost_gen", "ost_idx");

	int i;
	for (i = 0; i < lum->lmm_stripe_count; i++) {
		const struct lov_user_ost_data_v1 *o = &objs[i];
		printf("%12lx %12lx %8x %8x\n",
		       (unsigned long) o->l_object_id,
		       (unsigned long) o->l_object_seq,
		       (unsigned) o->l_ost_gen,
		       (unsigned) o->l_ost_idx);
	}

	close(fd);
#endif

	return 0;
}
