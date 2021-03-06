#ifndef NN_FLAT_SET_H
#define NN_FLAT_SET_H

#include <stdbool.h>
#include <stddef.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

/*
 * A flat set is an always sorted list of numbers.
 */
struct flat_set
{
  int *data;
  size_t size;
  size_t capacity;
};

struct flat_set make_flat_set(int capacity);
void destroy_flat_set(struct flat_set *flat_set);

void flat_set_clear(struct flat_set *flat_set);
bool flat_set_contains(struct flat_set *flat_set, int i);
size_t flat_set_lower_bound(struct flat_set *flat_set, int i);
size_t flat_set_insert(struct flat_set *flat_set, int i);
size_t flat_set_remove(struct flat_set *flat_set, int i);

#ifndef NDEBUG
void flat_set_print(struct flat_set *flat_set, FILE *fp);
#endif

#endif
