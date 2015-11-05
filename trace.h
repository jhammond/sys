#ifndef _TRACE_H_
#define _TRACE_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

extern const char *program_usage;

static inline void usage(FILE *file, int status)
{
  fprintf(file, "Usage: %s %s\n",
	  program_invocation_short_name, program_usage);
  exit(status);
}

static inline void bad_option(void)
{
  fprintf(stderr, "Try '%s --help' for more information\n",
	  program_invocation_short_name);
  exit(EXIT_FAILURE);
}

#define ERROR(fmt,args...)						\
  fprintf(stderr, "%s: " fmt, program_invocation_short_name, ##args)

#define FATAL(fmt,args...)			\
  do {						\
    ERROR(fmt, ##args);				\
    exit(EXIT_FAILURE);				\
  } while (0)

#endif
