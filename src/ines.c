#include "ines.h"

#include "util.h"

/*
 * Constructs an iNes header from the given ROM header data.
 */
struct iNesHeader make_ines_header(uint8_t header_data[16])
{
  struct iNesHeader header = {0};
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
  header.mapper = (header_data[7] & 0xf0) + ((header_data[6] & 0xf0) >> 4);

  return header;
}

/*
 * Calculates the offset of the PRG data (program data) in the given ROM data
 * given an iNes ROM header. Returns a pointer to the PRG data in `prg_data`.
 */
void iNesHeader_prg_data(struct iNesHeader header, uint8_t* rom_data,
                         uint8_t** prg_data)
{
  *prg_data =
      rom_data + (sizeof(struct iNesHeader) + (header.has_trainer ? 512 : 0));
}

/*
 * Returns the ROM size in bytes for the given iNes header.
 */
uint32_t iNesHeader_rom_size_in_bytes(struct iNesHeader* header)
{
  return header->prg_rom_size * 16 * 1024;
}

void write_rom_information(FILE* fp, uint8_t* rom_data)
{
  enum RomFormat rf = get_rom_format(rom_data);
  if (rf == RomFormat_Unknown)
  {
    quit("Unknown ROM file format, supported formats: iNES");
  }

  if (rf == RomFormat_iNes)
  {
    struct iNesHeader header = make_ines_header(rom_data);

    const int left_width = 50;

    const char* string_fmt = "; %-*s %s\n";
    const char* kb_fmt = "; %-*s %dKB\n";

    fprintf(fp, string_fmt, left_width, "ROM format:", "iNes");

    fprintf(fp, kb_fmt, left_width, "PRG ROM size:", header.prg_rom_size * 16);
    fprintf(fp, kb_fmt, left_width, "CHR ROM size:", header.chr_rom_size * 8);
    fprintf(fp, string_fmt, left_width,
            "Cartridge contains battery backed PRG RAM:",
            header.has_battery_backed_vram ? "Yes" : "No");

    fprintf(fp, string_fmt, left_width,
            "Mirroring:", string_from_mirroring(header.mirroring));
    fprintf(fp, "; %-*s %s (%d)\n", left_width,
            "Mapper:", string_from_mapper(header.mapper), header.mapper);
  }
  else if (rf == RomFormat_Nes20)
  {
    quit("NES 2.0 ROM format detected, unsupported for now");
  }

  /*
  struct RomHeader header;
  if (fread(&header, sizeof(struct RomHeader), 1, fp) != 1)
  {
    quit("Error while reading header from ROM");
  }

  if (strncmp(header.id, "NES", 3) != 0)
  {
    quit("Unknown ROM file format, supported formats: Archaic iNES, iNES,
  NES 2.0");
  }

  if (header.flags_7 && (header.flags_7 & 0xc) == 8)
  {
    printf("NES 2.0\n");
  }
  else if (header.flags_7 && (header.flags_7 & 0xc) == 0)
  {
    printf("iNES\n");
  }
  else
  {
    printf("Archaic iNES\n");
  }

  printf("; PRG ROM size:\t\t\t\t\t%dKB\n", header.prg_rom_size * 16);
  printf("; CHR ROM size:\t\t\t\t\t%dKB\n", header.chr_rom_size * 8);
  printf(";\n");
  printf("; Mirroring:\t\t\t\t\t%s\n", header.flags_6 & 0x1 ? "vertical" :
  "horizontal"); printf("; Cartridge contains battery packed memory:\t%s\n",
  header.flags_6 & 0x2 ? "yes" : "no"); printf("; Cartridge contains 512-byte
  trainer:\t\t%s\n", header.flags_6 & 0x4 ? "yes" : "no");

  int tv_mode = header.flags_10 & 0x3;
  printf(
      "; TV system:\t\t\t\t\t%s\n",
      tv_mode == 0 ? "NTSC" : (tv_mode == 2 ? "PAL" : "dual compatible"));
  printf("; PRG RAM size:\t\t\t\t\t%dKB\n", (header.flags_10 & 0x10 ?
  header.flags_8 : 1) * 8);
  */
}

enum RomFormat get_rom_format(uint8_t header[16])
{
  if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' &&
      header[3] == 0x1a)
  {
    return (header[7] & 0xc) == 0x8 ? RomFormat_Nes20 : RomFormat_iNes;
  }

  return RomFormat_Unknown;
}
