#include <lib/std/include/flat_set.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct flat_set make_flat_set(int capacity)
{
  struct flat_set fs = {malloc(sizeof(int) * capacity), 0, capacity};
  return fs;
}

/*
 * Frees dynamically allocated memory for a flat set object.
 */
void destroy_flat_set(struct flat_set *flat_set)
{
  free(flat_set->data);
}

/*
 * Clears the contents of the given flat set.
 */
void flat_set_clear(struct flat_set *flat_set)
{
  flat_set->size = 0;
}

/*
 * Returns whether the given number `n` is present in the set.
 */
bool flat_set_contains(struct flat_set *fs, int n)
{
  size_t i = flat_set_lower_bound(fs, n);
  return i != fs->size && fs->data[i] == n;
}

/*
 * Finds the first element in the set that is either equal to `n`, or greater
 * than `n`. In case no such number exists, returns the number of elements in
 * the set.
 */
size_t flat_set_lower_bound(struct flat_set *fs, int i)
{
  int *l = fs->data;
  int *h = fs->data + fs->size - 1;

  while (l <= h)
  {
    int *m = l + (h - l) / 2;
    if (*m < i)
    {
      l = m + 1;
    }
    else if (*m > i)
    {
      h = m - 1;
    }
    else
    {
      return m - fs->data;
    }
  }

  return l - fs->data;
}

/*
 * Inserts the given numbers in the set, and returns the index of the number in
 * the set. Returns the index of newly inserted number, or the index of the
 * number in the set in case it already existed.
 *
 * In case a number is inserted, and there is no memory available to hold it,
 * this will reallocate memory, reserving twice the amount of memory that was in
 * use before.
 */
size_t flat_set_insert(struct flat_set *fs, int n)
{
  /* The number may exist, but reallocate now anyway for simplicity. */
  if (fs->size == fs->capacity)
  {
    fs->capacity *= 2;
    fs->data = realloc(fs->data, sizeof(int) * fs->capacity);
  }

  const size_t i = flat_set_lower_bound(fs, n);
  int *p = fs->data + i;

  /* In case the number already exists, return its position. */
  if (i < fs->size && *p == n)
  {
    return i;
  }

  /* Insert the number. Create a new slot for the element to insert. */
  if (i < fs->size)
  {
    memmove(p + 1, p, sizeof(int) * (fs->size - i));
  }

  *p = n;
  ++fs->size;

  return i;
}

/*
 * Removes the given element in case it exists in the set. Returns the index of
 * the element that now comes after the removed element. Returns the size of
 * the set in case the element is not found, or was the last element in the set.
 */
size_t flat_set_remove(struct flat_set *flat_set, int n)
{
  const size_t i = flat_set_lower_bound(flat_set, n);

  if (i == flat_set->size || flat_set->data[i] != n)
  {
    return flat_set->size;
  }

  memmove(flat_set->data + i, flat_set->data + i + 1, sizeof(int) * (flat_set->size - i));
  flat_set->size--;

  return i;
}

#ifndef NDEBUG
void flat_set_print(struct flat_set *fs, FILE *fp)
{
  fprintf(fp, "flat_set: { ");
  int *first = fs->data;
  int *last = fs->data + fs->size;
  while (first != last)
  {
    fprintf(fp, "%d ", *first);
    ++first;
  }
  fprintf(fp, "}\n");
}
#endif
