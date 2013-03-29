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
	const char *path = argv[1];
	int fd;
	int rc = 0;

	fd = open(path, O_RDWR|O_CREAT|O_LOV_DELAY_CREATE, 0666);
	if (fd < 0)
		FATAL("cannot open '%s': %s\n",
		      path, strerror(errno));

	off_t size = 1048599;
	if (ftruncate(fd, size) < 0) {
		ERROR("cannot truncate '%s' to size %ld: %s\n",
		      path, size, strerror(errno));
		rc = 1;
	}

	return rc;
}
