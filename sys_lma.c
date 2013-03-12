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
#include "sys_lustre.h"
#include "trace.h"

const char *program_usage = "[OPTION]... PATH";

static int want_raw = 0;
static int want_hex = 0;

int main(int argc, char *argv[])
{
	const char *path;
	const char *attr_name = XATTR_NAME_LMA;
	size_t val_buf_size = XATTR_SIZE_MAX;
	char *val_buf = NULL;
	ssize_t val_len = 0;
	struct lustre_mdt_attrs *lma;

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

	if (val_len != sizeof(*lma))
		FATAL("size (%zd) for xattr '%s' of '%s' does not match "
		      "size (%zu) of lustre_mdt_attrs\n",
		      val_len, attr_name, path, sizeof(*lma));

	lma = (struct lustre_mdt_attrs *) val_buf;

#define X(w,m)						\
	printf("%-20s %24"PRIx##w"\n", #m, lma->m)

	X(32, lma_compat);
	/*
	 * Bitfield for supported data in this structure. From enum
	 * lma_compat.  lma_self_fid and lma_flags are always available.
	 */
	X(32, lma_incompat);
	/*
	 * Per-file incompat feature list. Lustre version should support all
	 * flags set in this field. The supported feature mask is available
	 * in LMA_INCOMPAT_SUPP.
	 */

	char self_fid_buf[PRI_FID_LEN + 1];
	snprintf(self_fid_buf, sizeof(self_fid_buf),
		 PRI_FID, PRI_FID_ARGS(&lma->lma_self_fid));
	printf("%-20s %24s\n", "lma_self_fid", self_fid_buf);
#if 0
	X(64, lma_self_fid.f_seq);
	X(32, lma_self_fid.f_oid);
	X(32, lma_self_fid.f_ver);
#endif
	X(64, lma_flags);
#undef X

	free(val_buf);

	return 0;
}
