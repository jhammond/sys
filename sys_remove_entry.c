#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "lustre/lustre_user.h"
#include "trace.h"

int main(int argc, char *argv[])
{
	const char *dir_path = argv[1];
	const char *ent_name = argv[2];
	int dir_fd = -1;

	/* TODO Hold open ent_fd. */

	dir_fd = open(dir_path, O_RDONLY|O_DIRECTORY);
	if (dir_fd < 0)
		FATAL("cannot open '%s': %s\n",
		      dir_path, strerror(errno));

	int rc = ioctl(dir_fd, LL_IOC_REMOVE_ENTRY, ent_name);
	if (rc < 0) {
		FATAL("cannot remove entry '%s' from '%s': %s\n",
		      ent_name, dir_path, strerror(errno));
		rc = 1;
	}

	if (close(dir_fd) < 0) {
		ERROR("error closing '%s': %s\n",
		      dir_path, strerror(errno));
		rc = 1;
	}

	return rc;
}
