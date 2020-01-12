#include "opcode_test.h"

#include <check.h>

START_TEST(initial_state)
{
}
END_TEST

TCase* make_opcode_test_case(void)
{
  TCase* tc = tcase_create("Opcode test cases");
  tcase_add_test(tc, initial_state);
  return tc;
}
