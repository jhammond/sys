#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char **environ;

static void usage(FILE *file, int status)
{
	fprintf(file, "Usage: %s [OPTION]... PATH [ARG]...\n",
		program_invocation_short_name);
	exit(status);
}

int main(int argc, char *argv[])
{
	const char *path;
	int fd;
	int rc;

	struct option opts[] = {
		{ "help", 0, NULL, 'h' },
		{ NULL }
	};

	int c;
	while ((c = getopt_long(argc, argv, "h", opts, 0)) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, EXIT_SUCCESS);
			break;
		case '?':
			fprintf(stderr, "Try '%s --help' for more information\n",
				program_invocation_short_name);
			exit(EXIT_FAILURE);
		}
	}

	if (argc - optind < 1)
		usage(stderr, EXIT_FAILURE);

	path = argv[optind];

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "cannot open '%s': %s\n",
			path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	rc = fexecve(fd, argv + optind, environ);
	if (rc < 0) {
		fprintf(stderr, "cannot exec '%s': %s\n",
			path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_FAILURE);
}
