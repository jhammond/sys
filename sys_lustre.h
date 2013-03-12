#ifndef _SYS_LUSTRE_H_
#define _SYS_LUSTRE_H_
#include <inttypes.h>
#include <sys/ioctl.h>
#include "lustre/lustre_user.h"

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

/* The link ea holds 1 \a link_ea_entry for each hardlink */
#define LINK_EA_MAGIC 0x11EAF1DFUL

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

struct lustre_mdt_attrs {
	/*
	 * Bitfield for supported data in this structure. From enum
	 * lma_compat.  lma_self_fid and lma_flags are always available.
	 */
	uint32_t	lma_compat;
	/*
	 * Per-file incompat feature list. Lustre version should support all
	 * flags set in this field. The supported feature mask is available
	 * in LMA_INCOMPAT_SUPP.
	 */
	uint32_t	lma_incompat;
	/** FID of this inode */
	struct lu_fid	lma_self_fid;
	/** mdt/ost type, others */
	uint64_t	lma_flags;
};

#endif
