#ifndef _SYS_LUSTRE_H_
#define _SYS_LUSTRE_H_
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include "lustre/lustre_user.h"
#include "lustre/lustre_idl.h"

#define XATTR_NAME_LOV     "trusted.lov"
#define XATTR_NAME_LMA     "trusted.lma"
#define XATTR_NAME_LMV     "trusted.lmv"
#define XATTR_NAME_LINK    "trusted.link"
#define XATTR_NAME_FID     "trusted.fid"
#define XATTR_NAME_VERSION "trusted.version"

#define PRI_FID "[0x%"PRIx64":0x%"PRIx32":0x%"PRIx32"]"
#define PRI_FID_ARGS(f) \
	((uint64_t) (f)->f_seq), \
	((uint32_t) (f)->f_oid), \
	((uint32_t) (f)->f_ver)
#define PRI_FID_LEN (10 + 2 * (sizeof(uint64_t) + 2 * sizeof(uint32_t)))

#define SCN_FID "[%"SCNx64":%"SCNx32":%"SCNx32"]"
#define SCN_FID_ARGS(fid)			\
	((uint64_t *)&((fid)->f_seq)),		\
	((uint32_t *)&((fid)->f_oid)),		\
	((uint32_t *)&((fid)->f_ver))

/* The link ea holds 1 \a link_ea_entry for each hardlink */
#define LINK_EA_MAGIC 0x11EAF1DFUL

#if 0
struct link_ea_header {
	uint32_t leh_magic;
	uint32_t leh_reccount;
	uint64_t leh_len; /* total size */
	uint32_t padding1; /* future use */
	uint32_t padding2; /* future use */
};

/* Hardlink data is name and parent fid.  Stored in this crazy struct
 * for maximum packing and endian-neutrality. */
struct link_ea_entry {
	/* __u16 stored big-endian, unaligned */
	unsigned char	lee_reclen[2];
	unsigned char	lee_parent_fid[sizeof(struct lu_fid)];
	char		lee_name[0];
} __attribute__((packed));
#endif

static inline ssize_t sys_lov_user_md_size(const struct lov_user_md *lum)
{
	if (!(lum->lmm_magic == LOV_USER_MAGIC_V1 ||
	      lum->lmm_magic == LOV_USER_MAGIC_V3)) {
		errno = EINVAL;
		return -1;
	}

	return lov_user_md_size(lum->lmm_stripe_count, lum->lmm_magic);
}

static inline int fprint_lov_user_md(FILE *file,
				     const struct lov_user_md *lum, 
				     size_t lum_size)
{
	int rc, print_count = 0;

	if (lum_size < sizeof(*lum)) {
		errno = EINVAL;
		rc = -1;
		goto out;
	}

	rc = fprintf(file,
		     "lmm_magic         %12"PRIx32"\n"
		     "lmm_pattern       %12x\n"
		     "lmm_released      %12d\n"
		     "lmm_oi_id         %12"PRIx64"\n"
		     "lmm_oi_seq        %12"PRIx64"\n"
		     "lmm_stripe_size   %12x\n"
		     "lmm_stripe_count  %12x\n"
		     "lmm_layout_gen    %12x\n",
		     (uint32_t) lum->lmm_magic,
		     lov_pattern(lum->lmm_pattern),
		     (lum->lmm_pattern & LOV_PATTERN_F_RELEASED) != 0,
		     (uint64_t) lmm_oi_id((struct ost_id *)&lum->lmm_oi),
		     (uint64_t) lmm_oi_seq((struct ost_id *)&lum->lmm_oi),
		     lum->lmm_stripe_size,
		     (unsigned) lum->lmm_stripe_count,
		     (unsigned) lum->lmm_layout_gen);
	if (rc < 0)
		goto out;
	print_count += rc;

	rc = fprintf(file, "%12s %12s %8s %8s %s\n",
		     "object_id", "object_seq", "ost_gen", "ost_idx", "FID");
	if (rc < 0)
		goto out;
	print_count += rc;

	const struct lov_user_ost_data_v1 *objs;

	if (lum->lmm_magic == LOV_USER_MAGIC_V1) {
		objs = &((const struct lov_user_md_v1 *) lum)->lmm_objects[0];
	} else if (lum->lmm_magic == LOV_USER_MAGIC_V3) {
		objs = &((const struct lov_user_md_v3 *) lum)->lmm_objects[0];
	} else {
		errno = EINVAL;
		rc = -1;
		goto out;
	}

	/* TODO Print pool. */

	size_t objs_size = lum_size - ((const char *)objs - (const char *)lum);
	size_t obj_count = objs_size / sizeof(objs[0]);

	if (lum->lmm_stripe_count < obj_count)
		obj_count = lum->lmm_stripe_count;

	int i;
	for (i = 0; i < obj_count; i++) {
		const struct lov_user_ost_data_v1 *o = &objs[i];
		struct lu_fid fid;

		ostid_to_fid(&fid, (struct ost_id *)&o->l_ost_oi, o->l_ost_idx);

		rc = fprintf(file, "%12"PRIx64" %12"PRIx64" %8x %8x "DFID"\n",
			     (uint64_t)lmm_oi_id((struct ost_id *)&o->l_ost_oi),
			     (uint64_t)lmm_oi_seq((struct ost_id *)&o->l_ost_oi),
			     (unsigned)o->l_ost_gen,
			     (unsigned)o->l_ost_idx,
			     PFID(&fid));
		if (rc < 0)
			goto out;
		print_count += rc;
	}

	rc = print_count;
out:
	return rc;
}

#endif
