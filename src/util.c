#include "util.h"

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const int MAXLINE = 120;

static void quit_optional_strerror(bool append_strerror, const char *fmt,
                                   va_list ap);

/*
 * Converts a 16-bit number from little to big endian.
 */
uint16_t ltob_uint16(uint16_t i)
{
  return ((i & 0x0ff) << 8) | ((i & 0x0ff00) >> 8);
}

/*
 * Converts a 32-bit number from little to big endian.
 */
uint32_t ltob_uint32(uint32_t i)
{
  return ltob_uint16(i & 0xffff) << 16 | ltob_uint16((i >> 16) & 0xffff);
}

/*
 * This will concatenate the given string x and y, and returns the result.
 * Allocates enough memory to hold the resulting concatenated string, plus the
 * zero character. In case memory allocation fails, returns a null pointer.
 */
char *nn_strcat(const char *x, const char *y)
{
  char *result;
  if ((result = malloc(strlen(x) + strlen(y) + 1)) != NULL)
  {
    result[0] = '\0';
    strcat(result, x);
    strcat(result, y);
  }
  return result;
}

/*
 * Returns whether the given string ends with the specified needle string.
 */
bool nn_ends_with(const char *s, const char *needle)
{
  size_t s_len = strlen(s);
  size_t needle_len = strlen(needle);
  if (needle_len <= s_len)
  {
    return strncmp(s + (s_len - needle_len), needle, needle_len) == 0;
  }

  return false;
}

/*
 * Returns the string up to and including the final '/' for the given
 * null-terminated string. This will allocate a new string to hold the directory
 * name of the given filename. In case the file name does not contain a '/',
 * return '.'. In case memory allocation fails, returns a null pointer.
 */
char *nn_dirname(const char *filename)
{
  int pos = strlen(filename);
  while (--pos >= 0 && filename[pos] != '/')
    ;

  return pos >= 0 ? strndup(filename, pos + 1) : strndup(".", 1);
}

/*
 * Creates the given path with the given permission bits initialize by the mode
 * parameter. All components of the given path that do not exist are created. In
 * case path creation fails, returns a negative value, otherwise, zero is
 * returned. In case of an error, the path may only be partially created.
 */
int nn_mkdirs(const char *path, mode_t mode)
{
  int ret;

  size_t parent_path_len;
  char *sep = strrchr(path, '/');
  if (sep != NULL && (parent_path_len = sep - path) > 0)
  {
    char parent_path[parent_path_len + 1];
    strncpy(parent_path, path, parent_path_len);
    parent_path[parent_path_len] = '\0';

    if ((ret = nn_mkdirs(parent_path, mode)) < 0)
    {
      return ret;
    }
  }

  if ((ret = access(path, F_OK)) < 0)
  {
    ret = mkdir(path, mode);
  }

  return ret;
}

/*
 * Logs a message to stderr.
 */
void nn_log(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);
  fflush(stderr);  // in case stdout and stderr are the same

  va_end(ap);
}

/*
 * Print a message to stderr, and exits the application.
 */
void quit(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  quit_optional_strerror(false, fmt, ap);
  va_end(ap);

  kill(0, SIGTERM);
}

/*
 * Prints a message to stderr, appending the error message generated from the
 * current error code, and exits the application.
 */
void quit_strerror(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  quit_optional_strerror(true, fmt, ap);
  va_end(ap);

  kill(0, SIGTERM);
}

static void quit_optional_strerror(bool append_strerror, const char *fmt,
                                   va_list ap)
{
  char buf[MAXLINE];
  vsnprintf(buf, MAXLINE - 1, fmt, ap);
  if (append_strerror)
    snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, ": %s",
             strerror(errno));
  strcat(buf, "\n");

  fputs(buf, stderr);
  fflush(stderr);
}
