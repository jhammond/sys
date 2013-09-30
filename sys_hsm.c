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
#include "sys_lustre.h"
#include "trace.h"

int main(int argc, char *argv[])
{
	struct hsm_user_request *hur;
	const char *action_name;
	const char *dir_path = NULL;
	struct lu_fid fid;
	int dir_fd = -1;
	int action;
	int archive_id = 0;
	size_t hur_size;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s ACTION DIR FID\n",
			program_invocation_short_name);
		exit(1);
	}

	action_name = argv[1];
	dir_path = argv[2];
	sscanf(argv[3], SCN_FID, SCN_FID_ARGS(&fid));

	if (strcasestr(action_name, "archive") != NULL)
		action = HUA_ARCHIVE;
	else if (strcasestr(action_name, "release") != NULL)
		action = HUA_RELEASE;
	else if (strcasestr(action_name, "restore") != NULL)
		action = HUA_RESTORE;
	else if (strcasestr(action_name, "remove") != NULL)
		action = HUA_REMOVE;
	else if (strcasestr(action_name, "cancel") != NULL)
		action = HUA_CANCEL;
	else
		FATAL("unrecoqnized action '%s'\n", action_name);

	dir_fd = open(dir_path, O_RDONLY);
	if (dir_fd < 0)
		FATAL("cannot open '%s': %s'\n",
		      dir_path, strerror(errno));

	hur_size = offsetof(struct hsm_user_request, hur_user_item[1]);
	hur = malloc(hur_size);
        if (hur == NULL)
                FATAL("cannot allocate HUR: %s\n", strerror(errno));

        hur->hur_request.hr_action = action;
        hur->hur_request.hr_archive_id = archive_id;
        hur->hur_request.hr_flags = 0;
	hur->hur_request.hr_itemcount = 1;
	hur->hur_request.hr_data_len = 0;

	hur->hur_user_item[0].hui_fid = fid;
	hur->hur_user_item[0].hui_extent.length = -1;

	if (ioctl(dir_fd, LL_IOC_HSM_REQUEST, hur) < 0)
		FATAL("cannot do archive "PRI_FID": %s\n",
		      PRI_FID_ARGS(&fid), strerror(errno));

	free(hur);

	return 0;
}
