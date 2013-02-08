#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "lustre/lustre_user.h"
#include "trace.h"

const char *program_usage = "[OPTION]... PATH";

int main(int argc, char *argv[])
{
	const char *path = NULL;
	int fd = -1;
	struct lov_user_md_v3 *lum = NULL;
	size_t stripe_count = 2;
	size_t lum_size;
	int want_quiet = 0;

	struct option opts[] = {
		{ "count", 1, NULL, 'c' },
		{ "help",  0, NULL, 'h' },
		{ "quiet", 0, NULL, 'q' },
		{ NULL }
	};

	int c;
	while ((c = getopt_long(argc, argv, "c:hq", opts, 0)) != -1) {
		switch (c) {
		case 'c':
			stripe_count = strtoul(optarg, NULL, 0);
			break;
		case 'h':
			usage(stdout, EXIT_SUCCESS);
			break;
		case 'q':
			want_quiet = 1;
			break;
		case '?':
			bad_option();
			break;
		}
	}

	if (argc - optind < 1)
		usage(stderr, EXIT_FAILURE);

	path = argv[optind];

	fd = open(path, O_RDWR|O_CREAT|O_LOV_DELAY_CREATE, 0666);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);

	lum_size = offsetof(typeof(*lum), lmm_objects[stripe_count + 1]);
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
		FATAL("cannot set stripe info for '%s': %m\n", path);

	if (lum->lmm_magic != LOV_USER_MAGIC_V3)
		FATAL("'%s' unexpected stripe md magic %x, expected %x\n",
		      path, lum->lmm_magic, LOV_USER_MAGIC_V3);

	if (want_quiet)
		goto out;

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

out:
	if (close(fd) < 0)
		FATAL("error closing '%s': %s\n",
		      path, strerror(errno));

	return 0;
}
