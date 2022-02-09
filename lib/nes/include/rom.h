#ifndef NEPNES_NES_ROM_H
#define NEPNES_NES_ROM_H

#include "mapper.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * TODO
 */
enum Mirroring
{
  MIRRORING_HORIZONTAL = 0,
  MIRRORING_VERTICAL = 1,
  MIRRORING_FOUR_SCREEN
};

static inline const char *mirroring_to_string(enum Mirroring mirroring)
{
  static const char *strings[] = {[MIRRORING_HORIZONTAL] = "Horizontal",
                                  [MIRRORING_VERTICAL] = "Vertical",
                                  [MIRRORING_FOUR_SCREEN] = "Four-screen"};
  return strings[mirroring];
}

/*
 * CPU/PPU timing, indicates in which region the game was released.
 */
enum Tv
{
  TV_NTSC,
  TV_PAL,
  TV_DUAL
};

/* Converts a TV enumeration value to string. */
static inline const char *tv_to_string(enum Tv tv)
{
  static const char *strings[] = {[TV_NTSC] = "NTSC", [TV_PAL] = "PAL", [TV_DUAL] = "Dual"};
  return strings[tv];
}

/*
 * Enumeration of all console types that can be encoded in an iNes header.
 */
enum ConsoleType
{
  CT_NES_FAMICOM = 0x0,
  CT_VS_SYSTEM = 0x1,
  CT_PLAYCHOICE_10 = 0x2,
  CT_BIT_CORPORATION_CREATOR = 0x3,
  CT_VT01_MONOCHROME = 0x4,
  CT_VT01_RED_CYAN = 0x5,
  CT_VT02 = 0x6,
  CT_VT03 = 0x7,
  CT_VT09 = 0x8,
  CT_VT32 = 0x9,
  CT_VT369 = 0xa,
  CT_UM6578 = 0xb
};

/* Converts a console type enumeration value to string. */
static inline const char *console_type_to_string(enum ConsoleType console_type)
{
  static const char *strings[] = {[CT_NES_FAMICOM] = "NES / Famicom / Dendy",
                                  [CT_VS_SYSTEM] = "VS System",
                                  [CT_PLAYCHOICE_10] = "Playchoice-10",
                                  [CT_BIT_CORPORATION_CREATOR] = "Bit Corporation Creator",
                                  [CT_VT01_MONOCHROME] = "VT01 Monochrome",
                                  [CT_VT01_RED_CYAN] = "VT01 Red/Cyan",
                                  [CT_VT02] = "VT02",
                                  [CT_VT03] = "VT03",
                                  [CT_VT09] = "VT09",
                                  [CT_VT32] = "VT32",
                                  [CT_VT369] = "VT369",
                                  [CT_UM6578] = "UM6578"};
  return strings[console_type];
}

/*
 * Enumeration of supported ROM header formats. NES 2.0 is an extension of the
 * iNes ROM format.
 */
enum RomFormat
{
  RF_UNKNOWN,
  RF_INES,
  RF_NES20
};

struct RomInfo
{
  int prg_rom_size;  // size of PRG ROM (program memory) in 16KB units
  int chr_rom_size;  // size of CHR ROM (character data, graphics) in 8KB units

  enum Mirroring mirroring;
  int has_battery_backed_vram;
  int has_trainer;
  int has_bus_conflicts;
  int has_four_screen_vram_layout;
  enum Mapper rom_mapper;

  int is_vs_system_cartridge;

  enum Tv tv_system;

  /*
   * 76543210
   * ||||||||
   * |||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU
   * A11)
   * |||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU
   * A10)
   * ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or
   * other persistent memory
   * |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
   * ||||+---- 1: Ignore mirroring control or above mirroring bit; instead
   * provide four-screen VRAM
   * ++++----- Lower nybble of mapper number
   */
  uint8_t flags_6;
  /*
   * 76543210
   * ||||||||
   * |||||||+- VS Unisystem
   * ||||||+-- PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
   * ||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
   * ++++----- Upper nybble of mapper number
   */
  uint8_t flags_7;
  /*
   * 76543210
   * ||||||||
   * ++++++++- PRG RAM size
   */
  uint8_t flags_8;
  /*
   * 76543210
   * ||||||||
   * |||||||+- TV system (0: NTSC; 1: PAL)
   * +++++++-- Reserved, set to zero
   */
  uint8_t flags_9;
  /*
   * 76543210
   * ||  ||
   * ||  ++- TV system (0: NTSC; 2: PAL; 1/3: dual compatible)
   * |+----- PRG RAM ($6000-$7FFF) (0: present; 1: not present)
   * +------ 0: board has no bus conflicts; 1: Board has bus conflicts
   */
  uint8_t flags_10;

  uint8_t padding[5];
};

/* iNES / NES2 header information. */
struct RomHeader
{
  enum RomFormat rom_format;

  uint8_t prg_rom_size;  // PRG ROM size (program memory) in 16KB blocks
  uint8_t chr_rom_size;  // CHR ROM size (character data) in 8KB blocks

  enum Mirroring mirroring;
  bool has_battery_backed_vram;
  bool has_trainer;
  enum ConsoleType console_type;
  enum Mapper mapper;
};

struct RomHeader rom_make_header(uint8_t header_data[16]);
void rom_prg_data(struct RomHeader *header, uint8_t *rom_data, uint8_t **prg_data,
                  size_t *prg_data_size);

enum RomFormat rom_get_format(uint8_t rom_header[16]);
int write_rom_information(FILE *fp, uint8_t *rom_data);

#endif
