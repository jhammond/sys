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

#ifdef DEBUG
#define TRACE ERROR
#else
static inline void TRACE(char *fmt, ...) { }
#endif

#define ERROR(fmt,args...) \
    fprintf(stderr, "%s: "fmt, program_invocation_short_name, ##args)

#define FATAL(fmt,args...) \
  do { \
    ERROR(fmt, ##args);\
    exit(1);\
  } while (0)

void usage(FILE *file, int status)
{
	/* ... */
	exit(status);
}

int main(int argc, char *argv[])
{
	const char *path = argv[1];
	int is_dir = (argv[2][0] == 'd');
	int fd = -1;

	fd = open(path, is_dir ? O_RDONLY : O_RDWR);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);

	int rc = ioctl(fd, ioc, NULL);
	if (rc < 0)
		FATAL("ioctl '%s' failed on '%s': %m\n", ioc_name, path);

	close(fd);

	return 0;
}
