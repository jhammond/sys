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
	const char *path = argv[1];
	struct hsm_user_request *hur = NULL;
	int fd = -1;

	size_t nr_items = 1024;
	size_t data_size = 4096;

#define HUR_SIZE(nr_items, data_size)				      \
	(offsetof(struct hsm_user_request, hur_user_item[nr_items]) + \
	 data_size)

	size_t hur_size = HUR_SIZE(nr_items, data_size);
	ERROR("hur_size %zu\n", hur_size);

	hur = malloc(hur_size);
	if (hur == NULL)
		FATAL("cannot allocate hur of size %zu: %s\n",
		      hur_size, strerror(errno));

	/* HUA_NONE is not recognized by server. */

	hur->hur_request = (struct hsm_request) { 
		.hr_action = HUA_ARCHIVE,
		.hr_archive_id = 0,
		.hr_flags = 0,
		.hr_itemcount = nr_items,
		.hr_data_len = data_size,
	};

	memset(hur->hur_user_item, 0,
	       HUR_SIZE(nr_items, data_size) - sizeof(*hur));

	fd = open(path, O_RDONLY);
	if (fd < 0)
		FATAL("cannot open '%s': %m\n", path);

	int rc = ioctl(fd, LL_IOC_HSM_REQUEST, hur);
	if (rc < 0)
		FATAL("ioctl '%s' failed on '%s': %s\n",
		      "LL_IOC_HSM_REQUEST", path, strerror(errno));

	if (close(fd) < 0)
		ERROR("error closing '%s': %s\n",
		      path, strerror(errno));

	return 0;
}
