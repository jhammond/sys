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

int main(int argc, char *argv[])
{
	const char *path = argv[1];
	int fd, rc;

	fd = open(path, O_RDONLY);

	rc = ioctl(fd, LL_IOC_MDC_GETINFO, argv[2]);
	if (rc < 0)
		FATAL("cannot get stripe info for '%s': %m\n", path);

	close(fd);

	return 0;
}
