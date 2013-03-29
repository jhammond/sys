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

#define ERROR(fmt,args...)						\
	fprintf(stderr, "%s: " fmt, program_invocation_short_name, ##args)

#define FATAL(fmt,args...)			\
	do {					\
		ERROR(fmt, ##args);		\
		exit(EXIT_FAILURE);		\
	} while (0)

int main(int argc, char *argv[])
{
	const char *path[2] = {
		argv[1],
		argc < 3 ? argv[1] : argv[2],
	};
	int fd[2];
	struct stat st[2];
	int rc = 0;
	off_t size = 1;
	size_t stripe_count = 2;

	int i;
	for (i = 0; i < 2; i++) {
		fd[i] = open(path[i], 
			     O_RDWR|O_CREAT|(i == 0 ? O_LOV_DELAY_CREATE : O_TRUNC),
			     0666);
		if (fd[i] < 0)
			FATAL("cannot open '%s' (%d): %s\n",
			      path[i], i, strerror(errno));
	}

	if (getenv("LBUG") != NULL)
		goto lbug;

	/* If the fstat() on fd[0] is happens here then the ioctl()
	 * call below will return -EEXIST. */
	for (i = 0; i < 2; i++) {
		if (fstat(fd[i], &st[i]) < 0)
			FATAL("cannot stat '%s' (%d): %s\n",
			      path[i], i, strerror(errno));
	}

	if (st[0].st_ino != st[1].st_ino) {
		ERROR("ino mismatch %lu, %lu\n",
		      st[0].st_ino, st[1].st_ino);
		rc = 1;
	}

lbug:
	i = 1;
	if (ftruncate(fd[i], size) < 0) {
		ERROR("cannot truncate '%s' (%d) to size %ld: %s\n",
		      path[i], i, size, strerror(errno));
		rc = 1;
	}

	struct lov_user_md_v3 *lum;
 	size_t lum_size = sizeof(*lum) +
		stripe_count * sizeof(lum->lmm_objects[0]);

	lum = malloc(lum_size);
	memset(lum, 0, lum_size);

	lum->lmm_magic = LOV_USER_MAGIC_V3;
	lum->lmm_pattern = 0;
        lum->lmm_stripe_size = 0;
        lum->lmm_stripe_count = stripe_count;
	lum->lmm_stripe_offset = -1;

	i = 0;
	if (ioctl(fd[i], LL_IOC_LOV_SETSTRIPE, lum) < 0) {
		ERROR("cannot setstripe on '%s' (%d): %s\n",
		      path[i], i, strerror(errno));
		rc = 1;
	}

	for (i = 0; i < 2; i++) {
		if (fstat(fd[i], &st[i]) < 0)
			FATAL("cannot stat '%s' (%d): %s\n",
			      path[i], i, strerror(errno));
	}

	if (st[0].st_ino != st[1].st_ino) {
		ERROR("ino mismatch %lu, %lu\n",
		      st[0].st_ino, st[1].st_ino);
		rc = 1;
	}

	for (i = 0; i < 2; i++) {
		if (close(fd[i]) < 0) {
			ERROR("close '%s' (%d) failed: %s\n",
			      path[i], i, strerror(errno));
			rc = 1;
		}
	}

	return rc;
}
