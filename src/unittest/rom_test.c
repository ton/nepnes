#include "rom_test.h"

#include <nn/nes/rom.h>
#include <nn/std/io.h>
#include <nn/std/util.h>

#include <check.h>

#include <stdlib.h>

static uint8_t *load_rom(const char *rom_file_name)
{
  uint8_t *rom_data;
  size_t rom_size;

  const char *roms_path = "src/unittest/input/roms/";
  char *rom_file_path = nn_strcat(roms_path, rom_file_name);

  if (nn_read_all(rom_file_path, &rom_data, &rom_size) == -1)
  {
    free(rom_file_path);
    ck_abort_msg("Could not open the input ROM file '%s' for reading", rom_file_name);
  }

  free(rom_file_path);
  return rom_data;
}

START_TEST(test_bingo_get_rom_format)
{
  uint8_t *rom_data = load_rom("bingo.nes");

  enum RomFormat rf = rom_get_format(rom_data);
  ck_assert_int_eq(rf, RF_INES);

  free(rom_data);
}
END_TEST

START_TEST(test_bingo_make_rom_header)
{
  uint8_t *rom_data = load_rom("bingo.nes");

  struct RomHeader header = rom_make_header(rom_data);
  ck_assert_int_eq(header.rom_format, RF_INES);
  ck_assert_int_eq(header.prg_rom_size, 2);
  ck_assert_int_eq(header.chr_rom_size, 1);
  ck_assert_int_eq(header.has_battery_backed_vram, 0);
  ck_assert_int_eq(header.has_trainer, 0);
  ck_assert_int_eq(header.mapper, MAPPER_NROM);
  ck_assert_int_eq(header.mirroring, MIRRORING_HORIZONTAL);

  free(rom_data);
}
END_TEST

START_TEST(test_bingo_rom_prg_data)
{
  uint8_t *rom_data = load_rom("bingo.nes");

  struct RomHeader header = rom_make_header(rom_data);
  ck_assert_int_eq(header.rom_format, RF_INES);

  uint8_t *prg_data;
  size_t prg_data_size;
  rom_prg_data(&header, rom_data, &prg_data, &prg_data_size);
  ck_assert_int_eq(prg_data - rom_data, 16);

  free(rom_data);
}
END_TEST

START_TEST(test_fail368_get_rom_format)
{
  uint8_t *rom_data = load_rom("nes-test-roms/nrom368/fail368.nes");

  enum RomFormat rf = rom_get_format(rom_data);
  ck_assert_int_eq(rf, RF_NES20);

  free(rom_data);
}
END_TEST

START_TEST(test_fail368_make_rom_header)
{
  uint8_t *rom_data = load_rom("nes-test-roms/nrom368/fail368.nes");

  struct RomHeader header = rom_make_header(rom_data);
  ck_assert_int_eq(header.rom_format, RF_NES20);
  ck_assert_int_eq(header.prg_rom_size, 2);
  ck_assert_int_eq(header.chr_rom_size, 1);
  ck_assert_int_eq(header.has_battery_backed_vram, 0);
  ck_assert_int_eq(header.has_trainer, 0);
  ck_assert_int_eq(header.mapper, MAPPER_NROM);
  ck_assert_int_eq(header.mirroring, MIRRORING_VERTICAL);

  free(rom_data);
}
END_TEST

TCase *make_rom_test_case(void)
{
  TCase *tc = tcase_create("ROM test cases");
  tcase_add_test(tc, test_bingo_get_rom_format);
  tcase_add_test(tc, test_bingo_make_rom_header);
  tcase_add_test(tc, test_bingo_rom_prg_data);
  tcase_add_test(tc, test_fail368_get_rom_format);
  tcase_add_test(tc, test_fail368_make_rom_header);
  return tc;
}
