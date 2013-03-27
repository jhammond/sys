#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/xattr.h>
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
		argv[2],
	};
	int fd[2], i, rc = 0;

	const char *lov_xattr_name = "trusted.lov";
	unsigned char lov_xattr_buf[80];

	for (i = 0; i < 2; i++) {
		int flags = (i == 0) ? O_RDONLY
			: O_RDWR|O_CREAT|O_LOV_DELAY_CREATE;

		fd[i] = open(path[i], flags, 0666);
		if (fd[i] < 0)
			FATAL("cannot open '%s': %s\n",
			      path[i], strerror(errno));
	}

	ssize_t lov_xattr_size;
	lov_xattr_size = fgetxattr(fd[0], lov_xattr_name, lov_xattr_buf,
				   sizeof(lov_xattr_buf));

	if (lov_xattr_size < 0)
		FATAL("cannot get xattr '%s' on '%s': %s\n",
		      lov_xattr_name, path[0], strerror(errno));

	/* XATTR_CREATE, XATTR_REPLACE */

	if (fsetxattr(fd[1], lov_xattr_name, lov_xattr_buf,
		      lov_xattr_size, 0) < 0)
		FATAL("cannot set xattr '%s' on '%s': %s\n",
		      lov_xattr_name, path[1], strerror(errno));

	return rc;
}
