#include "ines_test.h"

#include "ines.h"
#include "io.h"
#include "util.h"

#include <check.h>

#include <stdlib.h>

static uint8_t* load_rom(const char* rom_file_name)
{
  uint8_t* rom_data;
  size_t rom_size;

  const char* roms_path = "src/unittest/input/roms/";
  char* rom_file_path = nn_strcat(roms_path, rom_file_name);

  if (read_all(rom_file_path, &rom_data, &rom_size) == -1)
  {
    free(rom_file_path);
    ck_abort_msg("Could not open the input ROM file '%s' for reading",
                 rom_file_name);
  }

  free(rom_file_path);
  return rom_data;
}

START_TEST(test_ines_get_rom_format)
{
  uint8_t* rom_data = load_rom("bingo.nes");

  enum RomFormat rf = get_rom_format(rom_data);
  ck_assert_int_eq(rf, RomFormat_iNes);

  free(rom_data);
}
END_TEST

START_TEST(test_make_ines_header_bingo)
{
  uint8_t* rom_data = load_rom("bingo.nes");

  struct iNesHeader header = make_ines_header(rom_data);
  ck_assert_int_eq(header.prg_rom_size, 32);
  ck_assert_int_eq(header.chr_rom_size, 8);
  ck_assert_int_eq(header.has_battery_backed_vram, 0);
  ck_assert_int_eq(header.has_trainer, 0);
  ck_assert_int_eq(header.mapper, Mapper_NROM);
  ck_assert_int_eq(header.mirroring, Mirroring_Horizontal);

  free(rom_data);
}
END_TEST

START_TEST(test_ines_header_prg_data_bingo)
{
  uint8_t* rom_data = load_rom("bingo.nes");

  struct iNesHeader header = make_ines_header(rom_data);

  uint8_t* prg_data;
  ines_header_prg_data(header, rom_data, &prg_data);
  ck_assert_int_eq(prg_data - rom_data, sizeof(struct iNesHeader));

  free(rom_data);
}
END_TEST

TCase* make_ines_test_case(void)
{
  TCase* tc = tcase_create("iNes test cases");
  tcase_add_test(tc, test_ines_get_rom_format);
  tcase_add_test(tc, test_make_ines_header_bingo);
  tcase_add_test(tc, test_ines_header_prg_data_bingo);
  return tc;
}
