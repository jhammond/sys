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
	const char *path1 = argv[1];
	const char *path2 = argv[2];
	int rc;

	while (1) {
		rc = rename(path1, path2);
		if (rc == 0)
			continue;

		if (errno != ENOENT)
			FATAL("cannot rename '%s' to '%s': %s\n",
			      path1, path2, strerror(errno));

		rc = mknod(path1, S_IFREG|0666, 0);
		if (rc == 0)
			continue;

		if (errno != EEXIST)
			FATAL("cannot create '%s': %s\n",
			      path1, strerror(errno));
	}
}
