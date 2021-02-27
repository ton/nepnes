#include "io.h"

#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <zip.h>

static int BUFFER_SIZE = 128 * 1024;

/*
 * Decompresses the file with the given index in the given zip archive. Returns
 * 0 in case inflating the file in the archive succeeded. Otherwise, returns -1.
 */
static int inflate_file_index(zip_t *zip, int index, uint8_t **data, size_t *size)
{
  int entries = zip_get_num_entries(zip, ZIP_FL_UNCHANGED);
  if (index >= entries)
  {
    return -1;
  }

  zip_file_t *zip_file = zip_fopen_index(zip, index, ZIP_FL_UNCHANGED);
  if (zip_file == NULL)
  {
    return -1;
  }

  *data = malloc(BUFFER_SIZE);
  *size = BUFFER_SIZE;

  uint8_t *offset = *data;
  int bytes_read = 0;

  while ((bytes_read = zip_fread(zip_file, offset, BUFFER_SIZE)) == BUFFER_SIZE)
  {
    *size += BUFFER_SIZE;
    *data = realloc(*data, *size);
    offset = *data + *size - BUFFER_SIZE;
  }

  if (bytes_read == -1)
  {
    return -1;
  }

  if (0 < bytes_read && bytes_read < BUFFER_SIZE)
  {
    /* Ensure the buffer holds exactly the number of bytes that were read. */
    *size -= (BUFFER_SIZE - bytes_read);
    *data = realloc(*data, *size);
  }

  return 0;
}

/*
 * Reads all data from the given input file. This will allocate enough data to
 * hold the data read from file. In case the file does not exist or can not be
 * opened, returns -1, otherwise, returns 0.
 */
int read_all(const char *file_name, uint8_t **data, size_t *size)
{
  if (access(file_name, F_OK) == -1)
  {
    return -1;
  }

  /* In case the ROM file is in a ZIP archive, inflate it. */
  zip_t *zip = NULL;
  if ((zip = zip_open(file_name, ZIP_RDONLY, NULL)) != NULL)
  {
    return inflate_file_index(zip, 0, data, size);
  }

  /* Otherwise, just open the file as a whole. */
  FILE *fp = fopen(file_name, "r");
  if (fp == NULL)
  {
    return -1;
  }

  /* Seek until the end of the file. */
  if (fseek(fp, 0, SEEK_END) != 0)
  {
    return -1;
  }

  /* Determine the file size. */
  long pos = ftell(fp);
  if (pos == -1)
  {
    *size = 0;
    return -1;
  }
  *size = pos;

  /* Reset read head to the start of the file stream. */
  if (fseek(fp, 0, SEEK_SET) != 0)
  {
    return -1;
  }

  /* Allocate data. */
  *data = malloc(*size);
  if (data == NULL)
  {
    return -1;
  }

  /* Read all data. */
  if (fread(*data, 1, *size, fp) != *size)
  {
    return -1;
  }

  return 0;
}
