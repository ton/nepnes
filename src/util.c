#include "util.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int MAXLINE = 120;

static void quit_optional_strerror(bool append_strerror, const char* fmt, va_list ap);

/*
 * This will concatenate the given string x and y, and returns the result.
 * Allocates enough memory to hold the resulting concatenated string, plus the
 * zero character. In case memory allocation fails, returns a null pointer.
 */
char* nn_strcat(const char* x, const char* y)
{
  char* result;
  if ((result = malloc(strlen(x) + strlen(y) + 1)) != NULL)
  {
    result[0] = '\0';
    strcat(result, x);
    strcat(result, y);
  }
  return result;
}

/*
 * Print a message to stderr, and exits the application.
 */
void quit(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  quit_optional_strerror(false, fmt, ap);
  va_end(ap);

  exit(1);
}

/*
 * Prints a message to stderr, appending the error message generated from the
 * current error code, and exits the application.
 */
void quit_strerror(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  quit_optional_strerror(true, fmt, ap);
  va_end(ap);

  exit(1);
}

static void quit_optional_strerror(bool append_strerror, const char* fmt, va_list ap)
{
  char buf[MAXLINE];
  vsnprintf(buf, MAXLINE - 1, fmt, ap);
  if (append_strerror)
    snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, ": %s", strerror(errno));
  strcat(buf, "\n");

  fflush(stdout);  // in case stdout and stderr are the same
  fputs(buf, stderr);
  fflush(NULL);  // flush all stdio streams

  exit(1);
}
