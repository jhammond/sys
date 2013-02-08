#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "lustre/lustre_user.h"

int main(int argc, char *argv[])
{
	const char *path1 = argv[1];
	const char *path2 = argv[2];

	struct lov_user_md_v3 *lum;
	size_t stripe_count = 1;
 	size_t lum_size = sizeof(*lum) +
		stripe_count * sizeof(lum->lmm_objects[0]);

	lum = malloc(lum_size);
	memset(lum, 0, lum_size);

	lum->lmm_magic = LOV_USER_MAGIC_V3;
	lum->lmm_pattern = 0;
        lum->lmm_stripe_size = 0;
        lum->lmm_stripe_count = stripe_count;
	lum->lmm_stripe_offset = -1;

	int fd1 = open(path1, O_RDWR|O_CREAT, 0666);
	if (fd1 < 0) {
		fprintf(stderr, "cannot open '%s': %s\n",
			path1, strerror(errno));
		exit(1);
	}

	unlink(path2);

	int fd2 = open(path2, O_RDWR|O_CREAT|O_LOV_DELAY_CREATE, 0666);
	if (fd2 < 0) {
		fprintf(stderr, "cannot open '%s': %s\n",
			path2, strerror(errno));
		exit(1);
	}

	if (rename(path1, path2) < 0) {
		fprintf(stderr, "cannot rename '%s' to '%s': %s\n",
			path1, path2, strerror(errno));
		exit(1);
	}

	ioctl(fd2, LL_IOC_LOV_SETSTRIPE, lum);
	/* Unreached. */

	return 0;
}
