#include "da_test.h"
#include "flat_set_test.h"
#include "opcode_test.h"
#include "rom_test.h"

#include <check.h>

#include <stdio.h>

int main(void)
{
  Suite *suite = suite_create("nepnes test suite");
  suite_add_tcase(suite, make_da_test_case());
  suite_add_tcase(suite, make_opcode_test_case());
  suite_add_tcase(suite, make_rom_test_case());
  suite_add_tcase(suite, make_flat_set_test_case());

  SRunner *sr = srunner_create(suite);
  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_set_log(sr, "test.log");
  srunner_set_xml(sr, "test.xml");
  srunner_run_all(sr, CK_VERBOSE);

  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed == 0 ? 0 : 1;
}
