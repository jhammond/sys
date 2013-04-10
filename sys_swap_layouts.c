#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "lustre/lustre_user.h"
#include "trace.h"

const char *program_usage = "[OPTION]... PATH1 PATH2";

int main(int argc, char *argv[])
{
	const char *path[2] = { NULL, NULL };
	int fd[2] = { -1, -1 };

	struct option opts[] = {
		{ "help",  0, NULL, 'h' },
		{ NULL }
	};

	int c;
	while ((c = getopt_long(argc, argv, "h", opts, 0)) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, EXIT_SUCCESS);
			break;
		case '?':
			bad_option();
			break;
		}
	}

	if (argc - optind < 2)
		usage(stderr, EXIT_FAILURE);

	path[0] = argv[optind];
	path[1] = argv[optind + 1];

	int i, n = 2;

	for (i = 0; i < n; i++) {
		fd[i] = open(path[i], O_WRONLY|O_LOV_DELAY_CREATE, 0666);
		if (fd[i] < 0)
			FATAL("cannot open '%s': %m\n", path[i]);
	}

	struct lustre_swap_layouts sl = {
		.sl_fd = fd[1],
		.sl_flags = 0,
		.sl_gid = 0,
	};

	int rc;
	rc = ioctl(fd[0], LL_IOC_LOV_SWAP_LAYOUTS, &sl);
	if (rc < 0)
		FATAL("cannot swap layouts of '%s' and '%s': %s\n",
		      path[0], path[1], strerror(errno));

	for (i = 0; i < n; i++)
		if (close(fd[i]) < 0)
			FATAL("error closing '%s': %s\n",
			      path[i], strerror(errno));

	return 0;
}
