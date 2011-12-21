#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static void usage(FILE *file, int status)
{
  fprintf(file, "Usage: %s [OPTION]... PATH\n",
	  program_invocation_short_name);
  exit(status);
}

int main(int argc, char *argv[])
{
  const char *path;
  mode_t mode = 0777;
  int want_pause = 0;
  unsigned int delay = 0;

  struct option opts[] = {
    { "help", 0, NULL, 'h' },
    { "mode", 1, NULL, 'm' },
    { "pause", 0, NULL, 'p' },
    { "sleep", 1, NULL, 's' },
    { NULL }
  };

  int c;
  while ((c = getopt_long(argc, argv, "hm:ps:", opts, 0)) != -1) {
    switch (c) {
    case 'h':
      usage(stdout, EXIT_SUCCESS);
      break;
    case 'm':
      mode = strtol(optarg, NULL, 0);
      break;
    case 'p':
      want_pause = 1;
      break;
    case 's':
      delay = strtoul(optarg, NULL, 0);
      break;
    case '?':
      fprintf(stderr, "Try `%s --help' for more information\n",
	      program_invocation_short_name);
      exit(EXIT_FAILURE);
    }
  }

  if (argc - optind < 1)
    usage(stderr, EXIT_FAILURE);

  path = argv[optind];

  if (mkdir(path, mode) < 0) {
    fprintf(stderr, "cannot mkdir `%s' with mode %04o: %m\n", path, mode);
    exit(EXIT_FAILURE);
  }

  if (want_pause)
    pause();

  if (delay > 0)
    sleep(delay);

  return 0;
}