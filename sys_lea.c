#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <linux/limits.h> /* XATTR_{LIST,NAME,SIZE}_MAX */
#include <endian.h>
#include "lustre/lustre_user.h"
#include "sys_lustre.h"
#include "trace.h"

const char *program_usage = "[OPTION]... PATH";

static int want_raw = 0;
static int want_hex = 0;

static inline uint32_t swab32(uint32_t x)
{
	return ((x & 0x000000ffUL) << 24) |
		((x & 0x0000ff00UL) << 8) |
		((x & 0x00ff0000UL) >> 8) |
		((x & 0xff000000UL) >> 24);
}

static inline uint64_t swab64(uint64_t x)
{
	uint32_t h = x >> 32;
	uint32_t l = x & 0xffffffffUL;
	return (((uint64_t) swab32(l)) << 32) | ((uint64_t) (swab32(h)));
}

int main(int argc, char *argv[])
{
	const char *path;
	const char *attr_name = XATTR_NAME_LINK;
	size_t val_buf_size = XATTR_SIZE_MAX;
	char *val_buf = NULL;
	ssize_t val_len = 0;
	struct link_ea_header *leh;

	struct option opts[] = {
		{ "help", 0, NULL, 'h' },
		{ "raw",  0, NULL, 'r' },
		{ "hex",  0, NULL, 'x' },
		{ NULL }
	};

	int c;
	while ((c = getopt_long(argc, argv, "hrx", opts, 0)) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, EXIT_SUCCESS);
			break;
		case 'r':
			want_raw = 1;
			break;
		case 'x':
			want_hex = 1;
			break;
		case '?':
			bad_option();
			break;
		}
	}

	if (argc - optind != 1)
		usage(stderr, EXIT_FAILURE);

	path = argv[optind];

	while (1) {
		char *buf = realloc(val_buf, val_buf_size);
		if (buf == NULL)
			FATAL("cannot allocate buffer for xattr '%s' of '%s':"
			      " %s\n",
			      attr_name, path, strerror(errno));
		val_buf = buf;

		val_len = getxattr(path, attr_name, val_buf, val_buf_size);
		if (val_len >= 0)
			break;

		if (errno != ERANGE)
			FATAL("cannot retrieve xattr '%s' of '%s': %s\n",
			      attr_name, path, strerror(errno));

		val_buf_size *= 2;
	}

	if (want_raw) {
		fwrite(val_buf, 1, val_len, stdout);
		exit(EXIT_SUCCESS);
	}

	if (val_len < sizeof(*leh))
		FATAL("size (%zd) for xattr '%s' of '%s' less than "
		      "size (%zu) of link_ea_header\n",
		      val_len, attr_name, path, sizeof(*leh));

	leh = (struct link_ea_header *) val_buf;
	if (leh->leh_magic == swab32(LINK_EA_MAGIC)) {
		leh->leh_magic = LINK_EA_MAGIC;
		leh->leh_reccount = swab32(leh->leh_reccount);
		leh->leh_len = swab64(leh->leh_len);
	}

	if (leh->leh_magic != LINK_EA_MAGIC)
		FATAL("xattr '%s' of '%s' has invalid magic %08"PRIx32"\n",
		      attr_name, path, leh->leh_magic);

	unsigned int i;
	char *p = (char *) (leh + 1);

	for (i = 0; i < leh->leh_reccount; i++) {
		struct link_ea_entry *lee;
		size_t rec_len;
		struct lu_fid pfid;
		const char *name;
		int name_len;

		/* TODO Check that lee is sane. */
		lee = (struct link_ea_entry *) p;

		rec_len = (lee->lee_reclen[0] << 8) | lee->lee_reclen[1];
		memcpy(&pfid, &lee->lee_parent_fid, sizeof(pfid));
		pfid.f_seq = be64toh(pfid.f_seq);
		pfid.f_oid = be32toh(pfid.f_oid);
		pfid.f_ver = be32toh(pfid.f_ver);
		name = lee->lee_name;
		name_len = rec_len - sizeof(*lee);

		printf(PRI_FID" '%.*s'\n", PRI_FID_ARGS(&pfid), name_len, name);

		p += rec_len;
	}

	free(val_buf);

	return 0;
}
