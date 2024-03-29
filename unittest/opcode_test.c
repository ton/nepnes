#include "opcode_test.h"

#include <lib/6502/include/instruction.h>

#include <check.h>

START_TEST(initial_state)
{
  struct Instruction ins = {0};
  ck_assert_int_eq(0, ins.opcode);
}
END_TEST

TCase *make_opcode_test_case(void)
{
  TCase *tc = tcase_create("Opcode test cases");
  tcase_add_test(tc, initial_state);
  return tc;
}
