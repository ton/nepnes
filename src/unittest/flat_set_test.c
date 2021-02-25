#include "flat_set_test.h"

#include "tools/dbg/flat_set.h"

#include <check.h>

START_TEST(test_make_flat_set)
{
  const int capacity = 16;
  struct flat_set fs = make_flat_set(capacity);
  ck_assert_int_eq(fs.capacity, 16);
  ck_assert_int_eq(fs.size, 0);
  destroy_flat_set(&fs);
}
END_TEST

START_TEST(test_insert)
{
  const int capacity = 2;
  struct flat_set set = make_flat_set(capacity);
  ck_assert_int_eq(set.size, 0);

  /* Inserting a number that does not exist in the set should increase the size
   * of the set, whereas capacity remains unchanged. */
  flat_set_insert(&set, 3);
  ck_assert_int_eq(set.size, 1);
  ck_assert_int_eq(set.capacity, capacity);

  /* Inserting the same number twice should not change the size of the set,
   * whereas the capacity remains unchanged. */
  flat_set_insert(&set, 3);
  ck_assert_int_eq(set.size, 1);
  ck_assert_int_eq(set.capacity, capacity);

  /* Inserting a number that does not yet exist in the set should again increase
   * the size of the set, whereas the capacity remains unchanged. */
  flat_set_insert(&set, 4);
  ck_assert_int_eq(set.size, 2);
  ck_assert_int_eq(set.capacity, capacity);

  /* Now, inserting another number should allocate memory. */
  flat_set_insert(&set, 5);
  ck_assert_int_eq(set.size, 3);
  ck_assert_int_eq(set.capacity, 2 * capacity);

  /* Test inserting the same number twice again. */
  flat_set_insert(&set, 5);
  ck_assert_int_eq(set.size, 3);
  ck_assert_int_eq(set.capacity, 2 * capacity);

  destroy_flat_set(&set);
}
END_TEST

START_TEST(test_insert_out_of_order)
{
  const int capacity = 8;
  struct flat_set set = make_flat_set(capacity);
  ck_assert_int_eq(set.size, 0);

  /* Test inserting a bunch of numbers out of order. */
  ck_assert_int_eq(0, flat_set_insert(&set, 5));
  ck_assert_int_eq(set.size, 1);
  ck_assert_int_eq(0, flat_set_insert(&set, 4));
  ck_assert_int_eq(set.size, 2);
  ck_assert_int_eq(0, flat_set_insert(&set, 3));
  ck_assert_int_eq(set.size, 3);
  ck_assert_int_eq(0, flat_set_insert(&set, 2));
  ck_assert_int_eq(set.size, 4);
  ck_assert_int_eq(0, flat_set_insert(&set, 1));
  ck_assert_int_eq(set.size, 5);

  /* Now, add the numbers again, size should not change. */
  ck_assert_int_eq(0, flat_set_insert(&set, 1));
  ck_assert_int_eq(set.size, 5);
  ck_assert_int_eq(1, flat_set_insert(&set, 2));
  ck_assert_int_eq(set.size, 5);
  ck_assert_int_eq(2, flat_set_insert(&set, 3));
  ck_assert_int_eq(set.size, 5);
  flat_set_insert(&set, 4);
  ck_assert_int_eq(set.size, 5);
  flat_set_insert(&set, 5);
  ck_assert_int_eq(set.size, 5);

  /* The numbers should be present. */
  ck_assert_int_eq(0, flat_set_lower_bound(&set, 1));
  ck_assert_int_eq(1, flat_set_lower_bound(&set, 2));
  ck_assert_int_eq(2, flat_set_lower_bound(&set, 3));
  ck_assert_int_eq(3, flat_set_lower_bound(&set, 4));
  ck_assert_int_eq(4, flat_set_lower_bound(&set, 5));

  /* Validate the capacity. */
  ck_assert_int_eq(set.capacity, 8);

  destroy_flat_set(&set);
}
END_TEST

START_TEST(test_clear)
{
  /* Create a non-empty flat set. */
  struct flat_set set = make_flat_set(8);
  flat_set_insert(&set, 3);
  flat_set_insert(&set, 4);
  flat_set_insert(&set, 5);
  ck_assert_int_eq(set.size, 3);

  /* Clearing it should reset its size to zero. */
  flat_set_clear(&set);
  ck_assert_int_eq(false, flat_set_contains(&set, 3));
  ck_assert_int_eq(false, flat_set_contains(&set, 4));
  ck_assert_int_eq(false, flat_set_contains(&set, 5));
  ck_assert_int_eq(set.size, 0);
}
END_TEST

START_TEST(test_contains)
{
  struct flat_set set = make_flat_set(16);
  flat_set_insert(&set, 2);
  flat_set_insert(&set, 4);
  flat_set_insert(&set, 8);
  flat_set_insert(&set, 16);
  flat_set_insert(&set, 32);
  flat_set_insert(&set, 64);

  ck_assert_int_eq(true, flat_set_contains(&set, 2));
  ck_assert_int_eq(true, flat_set_contains(&set, 4));
  ck_assert_int_eq(true, flat_set_contains(&set, 8));
  ck_assert_int_eq(true, flat_set_contains(&set, 16));
  ck_assert_int_eq(true, flat_set_contains(&set, 32));
  ck_assert_int_eq(true, flat_set_contains(&set, 64));

  ck_assert_int_eq(false, flat_set_contains(&set, 0));
  ck_assert_int_eq(false, flat_set_contains(&set, 1));
  ck_assert_int_eq(false, flat_set_contains(&set, 3));
  ck_assert_int_eq(false, flat_set_contains(&set, 5));

  destroy_flat_set(&set);
}
END_TEST

START_TEST(test_lower_bound)
{
  struct flat_set set = make_flat_set(16);
  flat_set_insert(&set, 2);
  flat_set_insert(&set, 4);
  flat_set_insert(&set, 8);
  flat_set_insert(&set, 16);
  flat_set_insert(&set, 32);
  flat_set_insert(&set, 64);

  ck_assert_int_eq(0, flat_set_lower_bound(&set, 2));
  ck_assert_int_eq(1, flat_set_lower_bound(&set, 4));
  ck_assert_int_eq(2, flat_set_lower_bound(&set, 8));
  ck_assert_int_eq(3, flat_set_lower_bound(&set, 16));
  ck_assert_int_eq(4, flat_set_lower_bound(&set, 32));
  ck_assert_int_eq(5, flat_set_lower_bound(&set, 64));

  ck_assert_int_eq(0, flat_set_lower_bound(&set, 0));
  ck_assert_int_eq(0, flat_set_lower_bound(&set, 1));
  ck_assert_int_eq(1, flat_set_lower_bound(&set, 3));
  ck_assert_int_eq(2, flat_set_lower_bound(&set, 5));
  ck_assert_int_eq(set.size, flat_set_lower_bound(&set, 128));

  destroy_flat_set(&set);
}
END_TEST

START_TEST(test_remove)
{
  struct flat_set set = make_flat_set(16);
  flat_set_insert(&set, 2);
  flat_set_insert(&set, 4);
  flat_set_insert(&set, 8);
  flat_set_insert(&set, 16);
  flat_set_insert(&set, 32);
  flat_set_insert(&set, 64);

  ck_assert_int_eq(set.size, flat_set_remove(&set, 3));
  ck_assert_int_eq(6, set.size);

  ck_assert_int_eq(3, flat_set_remove(&set, 16));
  ck_assert_int_eq(5, set.size);

  ck_assert_int_eq(0, flat_set_remove(&set, 2));
  ck_assert_int_eq(4, set.size);

  ck_assert_int_eq(0, flat_set_remove(&set, 4));
  ck_assert_int_eq(3, set.size);

  ck_assert_int_eq(0, flat_set_remove(&set, 8));
  ck_assert_int_eq(2, set.size);

  ck_assert_int_eq(1, flat_set_remove(&set, 64));
  ck_assert_int_eq(1, set.size);

  ck_assert_int_eq(1, flat_set_remove(&set, 128));
  ck_assert_int_eq(1, set.size);

  ck_assert_int_eq(0, flat_set_remove(&set, 32));
  ck_assert_int_eq(0, set.size);

  destroy_flat_set(&set);
}
END_TEST

TCase *make_flat_set_test_case(void)
{
  TCase *flat_set = tcase_create("flat_set");
  tcase_add_test(flat_set, test_make_flat_set);
  tcase_add_test(flat_set, test_insert);
  tcase_add_test(flat_set, test_insert_out_of_order);
  tcase_add_test(flat_set, test_clear);
  tcase_add_test(flat_set, test_contains);
  tcase_add_test(flat_set, test_lower_bound);
  tcase_add_test(flat_set, test_remove);
  return flat_set;
}
