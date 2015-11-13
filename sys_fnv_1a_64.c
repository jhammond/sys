#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include "sys_lustre.h"
#include "trace.h"

const char *program_usage = "[OPTION]... STR";

int main(int argc, char *argv[])
{
	struct option opts[] = {
		{ NULL }
	};

	int c;
	while ((c = getopt_long(argc, argv, "", opts, 0)) != -1) {
		switch (c) {
		case '?':
			bad_option();
			break;
		}
	}

	if (argc - optind < 1)
		usage(stderr, EXIT_FAILURE);

	const char *str = argv[1];

	__u64 hash = lustre_hash_fnv_1a_64(str, strlen(str));

	printf(LPX64"\n", hash);

	return 0;
}
