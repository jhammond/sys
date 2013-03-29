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
#include "trace.h"

int main(int argc, char *argv[])
{
	const char *path = argv[1];
	int rc = 0;
	off_t len = 1;

	struct lov_user_md_v3 *lum;
	size_t stripe_count = 2;
 	size_t lum_size = sizeof(*lum) +
		stripe_count * sizeof(lum->lmm_objects[0]);

	lum = malloc(lum_size);
	memset(lum, 0, lum_size);

	lum->lmm_magic = LOV_USER_MAGIC_V3;
	lum->lmm_pattern = 0;
        lum->lmm_stripe_size = 0;
        lum->lmm_stripe_count = stripe_count;
	lum->lmm_stripe_offset = -1;

	int fd = open(path, O_RDWR|O_CREAT|O_LOV_DELAY_CREATE, 0666);
	if (fd < 0)
		FATAL("cannot open '%s': %s\n",
		      path, strerror(errno));

	if (truncate(path, len) < 0) {
		ERROR("cannot truncate '%s' to size '%ld': %s\n",
		      path, (long) len, strerror(errno));
		rc = 1;
	}

	if (ioctl(fd, LL_IOC_LOV_SETSTRIPE, lum) < 0) {
		ERROR("cannot setstripe on '%s': %s\n",
		      path, strerror(errno));
		rc = 1;
	}

	if (close(fd) < 0) {
		ERROR("close '%s' failed: %s\n",
		      path, strerror(errno));
		rc = 1;
	}

	return rc;
}
