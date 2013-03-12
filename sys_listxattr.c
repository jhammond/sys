#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <linux/limits.h> /* XATTR_{LIST,NAME,SIZE}_MAX */
#include "trace.h"

const char *program_usage = "[OPTION]... PATH";

static int want_raw = 0;
static int want_hex = 0;

static int fputc_esc(int c, FILE *file)
{
#if 0
	switch (c) {
#define X(c,e) case c: return fputs(e, file);
		X('\0', "\\0");
		X('\"', "\\\"");
		X('\\', "\\\\");
		X('\b', "\\b");
		X('\f', "\\f");
		X('\n', "\\n");
		X('\r', "\\r");
		X('\t', "\\t");
#undef X
	}

	if (c < 32 || c >= 127)
		return fprintf(file, "\\x%02x", (unsigned) c);
	else
		return fputc(c, file);
#else
	return fprintf(file, "%02x", (unsigned) c);
#endif
}

static int fprint_attr_val(FILE *file, const char *attr_name,
			   const char *val_buf, size_t val_len)
{
	const char *s;

	fprintf(file, "'%s' '", attr_name);

	for (s = val_buf; s < val_buf + val_len; s++)
		fputc_esc(*(const unsigned char *)s, file);

	fputs("'\n", file);

	return 0;
}

int main(int argc, char *argv[])
{
	const char *path;
	size_t list_buf_size = XATTR_LIST_MAX;
	char *list_buf = NULL;
	ssize_t list_len;
	size_t val_buf_size = XATTR_SIZE_MAX;
	char *val_buf = NULL;

	struct option opts[] = {
		{ "help", 0, NULL, 'h' },
		{ "raw",  0, NULL, 'r' },
		{ "hex",  0, NULL, 'x' },
		{ NULL }
	};

	int c;
	while ((c = getopt_long(argc, argv, "hrx", opts, 0)) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, EXIT_SUCCESS);
			break;
		case 'r':
			want_raw = 1;
			break;
		case 'x':
			want_hex = 1;
			break;
		case '?':
			bad_option();
			break;
		}
	}

	if (argc - optind != 1)
		usage(stderr, EXIT_FAILURE);

	path = argv[optind];

	while (1) {
		char *buf = realloc(list_buf, list_buf_size);
		if (buf == NULL)
			FATAL("cannot allocate xattr list for '%s': %s\n",
			      path, strerror(errno));
		list_buf = buf;

		list_len = listxattr(path, list_buf, list_buf_size);
		if (list_len >= 0)
			break;

		if (errno != ERANGE)
			FATAL("cannot retrieve xattr list for '%s': %s\n",
			      path, strerror(errno));

		list_buf_size *= 2;
	}

	char *attr_name = list_buf;
	while (attr_name < list_buf + list_len) {
		ssize_t val_len = 0;
		while (1) {
			char *buf = realloc(val_buf, val_buf_size);
			if (buf == NULL) {
				ERROR("cannot allocate buffer for xattr '%s' "
				      "of '%s': %s\n",
				      attr_name, path, strerror(errno));
				goto next_attr;
			}
			val_buf = buf;

			val_len = getxattr(path, attr_name, val_buf, val_buf_size);
			if (val_len >= 0)
				break;

			if (errno != ERANGE) {
				ERROR("cannot retrieve xattr '%s' of '%s': %s\n",
				      attr_name, path, strerror(errno));
				goto next_attr;
			}

			val_buf_size *= 2;
		}

		fprint_attr_val(stdout, attr_name, val_buf, val_len);

	next_attr:
		attr_name += strlen(attr_name) + 1;
	}

	free(list_buf);
	free(val_buf);

	return 0;
}
