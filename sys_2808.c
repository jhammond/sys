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
	int rc = 0;
	int i;

	for (i = 0; i < 2; i++) {
		fd[i] = open(path[i], 
			     O_RDWR|(i == 0 ? O_CREAT|O_LOV_DELAY_CREATE : 0),
			     0666);
		if (fd[i] < 0)
			FATAL("cannot open '%s' (%d): %s\n",
			      path[i], i, strerror(errno));
	}

	off_t size = 1;
	i = 0;
	if (ftruncate(fd[i], size) < 0) {
		ERROR("cannot truncate '%s' (%d) to size %ld: %s\n",
		      path[i], i, size, strerror(errno));
		rc = 1;
	}

	return rc;
}
