#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include "lustre/lustre_user.h"

#define ERROR(fmt,args...)						\
	fprintf(stderr, "%s: " fmt, program_invocation_short_name, ##args)

#define FATAL(fmt,args...)			\
	do {					\
		ERROR(fmt, ##args);		\
		exit(EXIT_FAILURE);		\
	} while (0)

#define PRI_FID "[0x%llx:0x%"PRIx32":0x%"PRIx32"]"
#define PRI_FID_ARGS(f) (f)->f_seq, (f)->f_oid, (f)->f_ver

int main(int argc, char *argv[])
{
	const char *path[2] = {
		argv[1],
		argc < 3 ? argv[1] : argv[2],
	};
	int fd[2], rc = 0;
	off_t len = 420019;

	system("lctl clear");

	int i, n = 2;
	for (i = 0; i < n; i++) {
		fd[i] = open(path[i], 
			     O_RDWR|(i == 0 ? O_CREAT|O_LOV_DELAY_CREATE : 0),
			     0666);
		if (fd[i] < 0)
			FATAL("cannot open '%s' (%d): %s\n",
			      path[i], i, strerror(errno));

		system(i == 0 ? "lctl dk > /tmp/dk0" : "lctl dk > /tmp/dk1");
	}

#if 0
	struct lu_fid fid[2];
	for (i = 0; i < n; i++) {
		if (ioctl(fd[i], LL_IOC_PATH2FID, &fid[i]) < 0)
			FATAL("cannot get FID for '%s' (%d): %s\n",
			      path[i], i, strerror(errno));

		ERROR("'%s' (%d) FID "PRI_FID"\n",
		      path[i], i, PRI_FID_ARGS(&fid[i]));
	}
#endif

	i = 0;
	if (ftruncate(fd[i], len) < 0) {
		ERROR("cannot truncate '%s' (%d) to size '%ld': %s\n",
		      path[i], i, (long) len, strerror(errno));
		rc = 1;
	}

	for (i = 0; i < n; i++) {
		if (close(fd[i]) < 0) {
			ERROR("close '%s' (%d) failed: %s\n",
			      path[i], i, strerror(errno));
			rc = 1;
		}
	}

	return rc;
}
