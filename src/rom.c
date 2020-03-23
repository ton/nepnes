#include "rom.h"

#include "util.h"

/*
 * Constructs a ROM header from the given raw ROM header data.
 */
struct RomHeader rom_make_header(uint8_t header_data[16])
{
  struct RomHeader header = {0};

  header.rom_format = rom_get_format(header_data);

  header.prg_rom_size = header_data[4];
  header.chr_rom_size = header_data[5];

  if (header_data[6] & 0x8)
  {
    header.mirroring = Mirroring_FourScreen;
  }
  else
  {
    header.mirroring = (header_data[6] & 0x1);
  }
  header.has_battery_backed_vram = (header_data[6] & 0x2);
  header.has_trainer = (header_data[6] & 0x4);
  header.console_type = (header_data[7] & 0x3);
  /* Read extended console type information. */
  if (header.console_type == 0x3)
  {
    header.console_type = (header_data[13] & 0xf);
  }
  header.mapper = (header_data[7] & 0xf0) + ((header_data[6] & 0xf0) >> 4);

  return header;
}

/*
 * Calculates the offset of the PRG data (program data) in the given ROM data
 * given an iNes ROM header. Returns a pointer to the PRG data in `prg_data`,
 * and the size of the PRG data segment in `prg_data_size`.
 */
void rom_prg_data(struct RomHeader *header, uint8_t *rom_data,
                  uint8_t **prg_data, size_t *prg_data_size)
{
  /* Header size is 16 bytes. */
  *prg_data = rom_data + (16 + (header->has_trainer ? 512 : 0));
  /* PRG ROM size is stored in units of 16KB blocks. */
  *prg_data_size = header->prg_rom_size * 16 * 1024;
}

/*
 * Writes ROM meta information to the given file pointer. Returns 0 in case
 * writing ROM meta information was successful, or an non-zero value in case of
 * an error.
 */
int write_rom_information(FILE *fp, uint8_t *rom_data)
{
  struct RomHeader header = rom_make_header(rom_data);
  if (header.rom_format == RomFormat_Unknown)
  {
    return 1;
  }

  const int left_width = 50;
  const char *string_fmt = "; %-*s %s\n";
  const char *kb_fmt = "; %-*s %dKB\n";

  fprintf(fp, string_fmt, left_width, "ROM format:",
          header.rom_format == RomFormat_iNes ? "iNes" : "NES 2.0");

  fprintf(fp, kb_fmt, left_width, "PRG ROM size:", header.prg_rom_size * 16);
  fprintf(fp, kb_fmt, left_width, "CHR ROM size:", header.chr_rom_size * 8);
  fprintf(fp, string_fmt, left_width,
          "Cartridge contains battery backed PRG RAM:",
          header.has_battery_backed_vram ? "Yes" : "No");

  fprintf(fp, string_fmt, left_width,
          "Mirroring:", mirroring_to_string(header.mirroring));
  fprintf(fp, string_fmt, left_width,
          "Console type:", console_type_to_string(header.console_type));
  fprintf(fp, "; %-*s %s (%d)\n", left_width,
          "Mapper:", mapper_to_string(header.mapper), header.mapper);

  return ferror(fp);
}

enum RomFormat rom_get_format(uint8_t header[16])
{
  if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' &&
      header[3] == 0x1a)
  {
    return (header[7] & 0xc) == 0x8 ? RomFormat_Nes20 : RomFormat_iNes;
  }

  return RomFormat_Unknown;
}
