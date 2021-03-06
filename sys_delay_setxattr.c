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
	const char *path = argv[1];
	int fd, rc = 0;

	const char *lov_xattr_name = "trusted.lov";
	unsigned char lov_xattr_value[] = {
		0xd0, 0x0b, 0xd1, 0x0b, 0x01, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x04, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	fd = open(path, O_RDWR|O_CREAT|O_LOV_DELAY_CREATE, 0666);
	if (fd < 0)
		FATAL("cannot open '%s': %s\n",
		      path, strerror(errno));

	/* XATTR_CREATE, XATTR_REPLACE */

	if (fsetxattr(fd, lov_xattr_name,
		      lov_xattr_value, sizeof(lov_xattr_value), 0) < 0)
		FATAL("cannot set xattr '%s' on '%s': %s\n",
		      lov_xattr_name, path, strerror(errno));

	return rc;
}
