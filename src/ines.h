#ifndef NEPNES_INES_H
#define NEPNES_INES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* clang-format off */
enum Mapper
{
  Mapper_NROM = 0,                    // All 32kB ROM + 8kB VROM games
  Mapper_MMC1 = 1,                    // Nintendo MMC1 (Megaman2, Bomberman2, etc.)
  Mapper_UXROM = 2,                   // UNROM/UOROM/etc. switch (QBert, PipeDream, Cybernoid,
                                      // many Japanese games)
  Mapper_CNROM = 3,                   // CNROM switch (Castlevania, LifeForce, etc.)
  Mapper_MMC3 = 4,                    // Nintendo MMC3 (SilverSurfer, SuperContra, Immortal, etc.)
  Mapper_MMC5 = 5,                    // Nintendo MMC5 (Castlevania3)
  Mapper_F4XXX = 6,                   // FFE F4xxx (F4xxx games off FFE CDROM)
  Mapper_AxROM = 7,                   // AOROM switch (WizardsAndWarriors, Solstice, etc.)
  Mapper_F3XXX = 8,                   // FFE F3xxx (F3xxx games off FFE CDROM)
  Mapper_MMC2 = 9,                    // Nintendo MMC2 (Punchout)
  Mapper_MMC4I = 10,                  // Nintendo MMC4 (Punchout2)
  Mapper_COLOR_DREAMS = 11,           // ColorDreams chip (CrystalMines, TaginDragon, etc.)
  Mapper_F6XXX = 12,                  // FFE F6xxx (F6xxx games off FFE CDROM)
  Mapper_CPROM = 13,                  // CPROM switch
  Mapper_REX_SL1632 = 14,             // SL-1632 PCB (8-character version of Samurai Spirits)
  Mapper_100_IN_1 = 15,               // 100-in-1 switch
  Mapper_BANDAI = 16,                 // Bandai chip (Japanese DragonBallZ series, etc.)
  Mapper_F8XXX = 17,                  // FFE F8xxx (F8xxx games off FFE CDROM)
  Mapper_JALECO = 18,                 // Jaleco SS8806 chip (Japanese Baseball3, etc.)
  Mapper_NAMCOT = 19,                 // Namcot 106 chip (Japanese GhostHouse2, Baseball90, etc.)
  Mapper_VRC2_VCR4_REV_A = 21,        // Konami VRC2/VCR4 rev. a (Japanese WaiWaiWorld2, etc.)
  Mapper_VRC2_VCR4_REV_B = 22,        // Konami VRC2/VCR4 rev. b (Japanese TwinBee3)
  Mapper_VRC2_VCR4_REV_C = 23,        // Konami VRC2/VCR4 rev. c (Japanese WaiWaiWorld)
  Mapper_VRC6_REV_A = 24,             // Konami VRC6 rev. a
  Mapper_VRC2_VCR4_REV_D = 25,        // Konami VRC2/VCR4 rev. d
  Mapper_VRC6_REV_B = 26,             // Konami VRC6 rev. b
  Mapper_VCR4_PIRATE = 27,            // Believed to be a pirate variant of VCR4
  Mapper_ACTION_53 = 28,              // Multicart discrete mapper
  Mapper_RET_CUFROM = 29,             // RET-CUFROM
  Mapper_UNROM512 = 30,               // UNROM 512 (Study Hall, Mystic Origins)
  Mapper_NSF = 31,                    //
  Mapper_IREM_G101 = 32,              // Irem G-101 chip (Japanese ImageFight, etc.)
  Mapper_TC0190FMC_TC0350FMR = 33,    //
  Mapper_IREM_BNROM = 34,             //
  Mapper_WARIO_LAND_2 = 35,           //
  Mapper_TXC_POLICEMAN = 36,          //
  Mapper_PAL_ZZ_SMB_TETRIS_NWC = 37,  //
  Mapper_BIT_CORP = 38,               //
  Mapper_SMB2J_FDS = 40,              //
  Mapper_CALTRON_6_in_1 = 41,         //
  Mapper_BIO_MIRACLE_FDS = 42,        //
  Mapper_FDS_SMB2J_LF36 = 43,         //
  Mapper_MMC3_BMC_PIRATE_REV_A = 44,  //
  Mapper_MMC3_BMC_PIRATE_REV_B = 45,  //
  Mapper_RUMBLESTATION_15_in_1 = 46,  //
  Mapper_NES_QJ_SSVB_NWC = 48,        //
  Mapper_MMC3_BMC_REV_C = 49,         //
  Mapper_SMB2j_FDS_REV_A = 50,        //
  Mapper_11_in_1_BALL_SERIES = 51,    //
  Mapper_MMC3_BMC_PIRATE_REV_D = 52,  //
  Mapper_SUPERVISION_16_IN_1 = 53,    //
  Mapper_SIMPLE_BMC_REV_A = 57,       //
  Mapper_SIMPLE_BMC_REV_B = 58,       //
  Mapper_SIMPLE_BMC_REV_C = 60,       //
  Mapper_20_IN_1_KAISER_REV_A = 61,   //
  Mapper_700_IN_1 = 62,               //
  Mapper_TENGEN = 64,                 //
  Mapper_IREM_H_3001 = 65,            //
  Mapper_MHROM = 66,                  //
  Mapper_SUNSOFT_FZII = 67,           //
  Mapper_SUNSOFT4 = 68,               //
  Mapper_SUNSOFT5_FME_7 = 69,         //
  Mapper_BA_KAMEN_DISCRETE = 70,      //
  Mapper_CAMERICA = 71,               // Camerica chip
  Mapper_JALECO_JF_17 = 72,           //
  Mapper_KONAMI_VRC3 = 73,            //
  Mapper_TW_MMC3_VRAM_REV_A = 74,     //
  Mapper_KONAMI_VRC1 = 75,            //
  Mapper_NAMCOT_108_REV_A = 76,       //
  Mapper_IREM_LROG017 = 77,           //
  Mapper_IREM = 78,                   // Irem 74HC161/32-based
  Mapper_NINA3 = 79,                  // AVE Nina-3 board (KrazyKreatures, DoubleStrike, etc.)
  Mapper_TAITO_X1_005 = 80,           //
  Mapper_NINA6 = 81,                  // AVE Nina-6 board (Deathbots, MermaidsOfAtlantis, etc.)
  Mapper_TAITO_X1_017 = 82,           //
  Mapper_YOKO_VRC_REV_B = 83,         //
  Mapper_KONAMI_VRC7 = 85,            //
  Mapper_JALECO_JF_13 = 86,           //
  Mapper_74x139_74_DISCRETE = 87,     //
  Mapper_NAMCO_3433 = 88,             //
  Mapper_SUN_SOFT_3 = 89,             //
  Mapper_HUMMER_JY = 90,              //
  Mapper_EARLY_HUMMER_JY = 91,        // Pirate HK-SF3 chip
  Mapper_JALECO_JF_19 = 92,           //
  Mapper_SUN_SOFT_3R = 93,            //
  Mapper_HVC_UN1ROM = 94,             //
  Mapper_NAMCO_108_REV_B = 95,        //
  Mapper_BANDAI_OEKAKIDS = 96,        //
  Mapper_IREM_TAM_S1 = 97,            //
  Mapper_VS_UNI_DUAL_SYSTEM = 99,     //
  Mapper_FDS_DOKIDOKI_FULL = 103,     //
  Mapper_NES_EVENT_NWC1990 = 105,     //
  Mapper_SMB3_PIRATE_A = 106,         //
  Mapper_MAGIC_CORP_A = 107,          //
  Mapper_FDS_UNROM_BOARD = 108,       //
  Mapper_CHEAPOCABRA = 111,           //
  Mapper_ASDER_NTDEC = 112,           //
  Mapper_HACKER_SACHEN = 113,         //
  Mapper_MMC3_SG_PROT_A = 114,        //
  Mapper_MMC3_PIRATE_A = 115,         //
  Mapper_MMC1_MMC3_VRC_PIRATE = 116,  //
  Mapper_FUTURE_MEDIA = 117,          //
  Mapper_TSK = 118,                   //
  Mapper_TQROM = 119,                 //
  Mapper_FDS_TOBIDASE = 120,          //
  Mapper_MMC3_PIRATE_PROT_A = 121,    //
  Mapper_MMC3_PIRATE_H2288 = 123,     //
  Mapper_FDS_LH32 = 125,              //
  Mapper_TXC_MGENIUS_22111 = 132,     //
  Mapper_SA72008 = 133,               //
  Mapper_MMC3_BMC_PIRATE = 134,       //
  Mapper_TCU02 = 136,                 //
  Mapper_S8259D = 137,                //
  Mapper_S8259B = 138,                //
  Mapper_S8259C = 139,                //
  Mapper_JALECO_JF_11_14 = 140,       //
  Mapper_S8259A = 141,                //
  Mapper_UNLKS7032 = 142,             //
  Mapper_TCA01 = 143,                 //
  Mapper_AGCI_50282 = 144,            //
  Mapper_SA72007 = 145,               //
  Mapper_SA0161M = 146,               //
  Mapper_TCU01 = 147,                 //
  Mapper_SA0037 = 148,                //
  Mapper_SA0036 = 149,                //
  Mapper_S74LS374N = 150,             //
  Mapper_BANDAI_SRAM = 153,           //
  Mapper_BANDAI_BARCODE = 157,        //
  Mapper_BANDAI_24C01 = 159,          //
  Mapper_SA009 = 160,                 //
  Mapper_SUBOR_rev_A = 166,           //
  Mapper_SUBOR_rev_B = 167,           //
  Mapper_BMCFK23C = 176,              //
  Mapper_TW_MMC3_VRAM_REV_B = 192,    //
  Mapper_NTDEC_TC_112 = 193,          //
  Mapper_TW_MMC3_VRAM_REV_C = 194,    //
  Mapper_TW_MMC3_VRAM_REV_D = 195,    //
  Mapper_TW_MMC3_VRAM_REV_E = 198,    //
  Mapper_NAMCOT_108_REV_C = 206,      //
  Mapper_TAITO_X1_005_REV_B = 207,    //
  Mapper_UNLA9746 = 219,              //
  Mapper_DEBUG_MAPPER = 220,          //
  Mapper_UNLN625092 = 221,            //
  Mapper_BMC_22_20_IN_1 = 226,        //
  Mapper_BMC_Contra_22_IN_1 = 230,    //
  Mapper_BMC_QUATTRO = 232,           //
  Mapper_BMC_22_20_IN_1_RST = 233,    //
  Mapper_BMC_MAXI = 234,              //
  Mapper_UNL6035052 = 238,            //
  Mapper_S74LS374NA = 243,            //
  Mapper_DECATHLON = 244,             //
  Mapper_FONG_SHEN_BANG = 246,        //
  Mapper_SAN_GUO_ZHI_PIRATE = 252,    //
  Mapper_DRAGON_BALL_PIRATE = 253,    //
};
/* clang-format on */

static inline const char *string_from_mapper(enum Mapper mapper)
{
  static const char *strings[] = {
      "NROM",                      // 0
      "MMC1",                      // 1
      "UxROM",                     // 2
      "CNROM",                     // 3
      "MMC3",                      // 4
      "MMC5",                      // 5
      "FFE Rev. A",                // 6
      "AxROM",                     // 7
      "",                          // 8
      "MMC2",                      // 9
      "MMC4",                      // 10
      "Color Dreams",              // 11
      "MMC3 Rev. A",               // 12
      "CPROM",                     // 13
      "Rex SL-1632",               // 14
      "100-in-1",                  // 15
      "BANDAI",                    // 16
      "FFE Rev. B",                // 17
      "Jaleco SS88006",            // 18
      "Namco 129/163",             // 19
      "",                          // 20
      "Konami VCR2/VCR4 rev. a",   // 21
      "Konami VCR2/VCR4 rev. b",   // 22
      "Konami VCR2/VCR4 rev. c",   // 23
      "Konami VCR6 rev. a",        // 24
      "Konami VCR2/VCR4 rev. d",   // 25
      "Konami VCR6 rev. b",        // 26
      "Pirated VCR4",              // 27
      "Action 53",                 // 28
      "RET-CUFORM",                // 29
      "UNROM 512",                 // 30
      "NSF",                       // 31
      "IREM G-101",                // 32
      "TC0190FMC/TC0350FMR",       // 33
      "IREM I-IM/BNROM",           // 34
      "Wario Land 2",              // 35
      "TXC Policeman",             // 36
      "PAL-ZZ SMB/TETRIS/NWC",     // 37
      "Bit Corp.",                 // 38
      "",                          // 39
      "SMB2j FDS",                 // 40
      "CALTRON 6-in-1",            // 41
      "BIO MIRACLE FDS",           // 42
      "FDS SMB2j LF36",            // 43
      "MMC3 BMC PIRATE rev. a",    // 44
      "MMC3 BMC PIRATE rev. b",    // 45
      "RUMBLESTATION 15-in-1",     // 46
      "",                          // 47
      "NES-QJ SSVB/NWC",           // 48
      "MMC3 BMC PIRATE rev. c",    // 49
      "SMB2j FDS rev. A",          // 50
      "11-in-1 BALL SERIES",       // 51
      "MMC3 BMC PIRATE rev. d",    // 52
      "SUPERVISION 16-in-1",       // 53
      "",                          // 54
      "",                          // 55
      "",                          // 56
      "SIMPLE BMC PIRATE rev. a",  // 57
      "SIMPLE BMC PIRATE rev. b",  // 58
      "",                          // 59
      "SIMPLE BMC PIRATE rev. c",  // 60
      "20-in-1 KAISER rev. a",     // 61
      "700-in-1",                  // 62
      "",                          // 63
      "TENGEN RAMBO1",             // 64
      "IREM H-3001",               // 65
      "MHROM",                     // 66
      "SUNSOFT-FZII",              // 67
      "SunSoft Mapper #4",         // 68
      "SUNSOFT-5/FME-7",           // 69
      "BA KAMEN DISCRETE",         // 70
      "CAMERICA BF9093",           // 71
      "JALECO JF-17",              // 72
      "KONAMI VRC3",               // 73
      "TW MMC3+VRAM Rev. A",       // 74
      "KONAMI VRC1",               // 75
      "NAMCOT 108 Rev. A",         // 76
      "IREM LROG017",              // 77
      "Irem 74HC161/32",           // 78
      "AVE/C&E/TXC BOARD",         // 79
      "TAITO X1-005 Rev. A",       // 80
      "",                          // 81
      "TAITO X1-017",              // 82
      "YOKO VRC Rev. B",           // 83
      "",                          // 84
      "KONAMI VRC7",               // 85
      "JALECO JF-13",              // 86
      "74*139/74 DISCRETE",        // 87
      "NAMCO 3433",                // 88
      "SUNSOFT-3",                 // 89
      "HUMMER/JY BOARD",           // 90
      "EARLY HUMMER/JY BOARD",     // 91
      "JALECO JF-19",              // 92
      "SUNSOFT-3R",                // 93
      "HVC-UN1ROM",                // 94
      "NAMCOT 108 Rev. B",         // 95
      "BANDAI OEKAKIDS",           // 96
      "IREM TAM-S1",               // 97
      "",                          // 98
      "VS Uni/Dual- system",       // 99
      "",                          // 100
      "",                          // 101
      "",                          // 102
      "FDS DOKIDOKI FULL",         // 103
      "",                          // 104
      "NES-EVENT NWC1990",         // 105
      "SMB3 PIRATE A",             // 106
      "MAGIC CORP A",              // 107
      "FDS UNROM BOARD",           // 108
      "",                          // 109
      "",                          // 110
      "Cheapocabra",               // 111
      "ASDER/NTDEC BOARD",         // 112
      "HACKER/SACHEN BOARD",       // 113
      "MMC3 SG PROT. A",           // 114
      "MMC3 PIRATE A",             // 115
      "MMC1/MMC3/VRC PIRATE",      // 116
      "FUTURE MEDIA BOARD",        // 117
      "TSKROM",                    // 118
      "NES-TQROM",                 // 119
      "FDS TOBIDASE",              // 120
      "MMC3 PIRATE PROT. A",       // 121
      "",                          // 122
      "MMC3 PIRATE H2288",         // 123
      "",                          // 124
      "FDS LH32",                  // 125
      "",                          // 126
      "",                          // 127
      "",                          // 128
      "",                          // 129
      "",                          // 130
      "",                          // 131
      "TXC/MGENIUS 22111",         // 132
      "SA72008",                   // 133
      "MMC3 BMC PIRATE",           // 134
      "",                          // 135
      "TCU02",                     // 136
      "S8259D",                    // 137
      "S8259B",                    // 138
      "S8259C",                    // 139
      "JALECO JF-11/14",           // 140
      "S8259A",                    // 141
      "UNLKS7032",                 // 142
      "TCA01",                     // 143
      "AGCI 50282",                // 144
      "SA72007",                   // 145
      "SA0161M",                   // 146
      "TCU01",                     // 147
      "SA0037",                    // 148
      "SA0036",                    // 149
      "S74LS374N",                 // 150
      "",                          // 151
      "",                          // 152
      "BANDAI SRAM",               // 153
      "",                          // 154
      "",                          // 155
      "",                          // 156
      "BANDAI BARCODE",            // 157
      "",                          // 158
      "BANDAI 24C01",              // 159
      "SA009",                     // 160
      "",                          // 161
      "",                          // 162
      "",                          // 163
      "",                          // 164
      "",                          // 165
      "SUBOR Rev. A",              // 166
      "SUBOR Rev. B",              // 167
      "",                          // 168
      "",                          // 169
      "",                          // 170
      "",                          // 171
      "",                          // 172
      "",                          // 173
      "",                          // 174
      "",                          // 175
      "BMCFK23C",                  // 176
      "",                          // 177
      "",                          // 178
      "",                          // 179
      "",                          // 180
      "",                          // 181
      "",                          // 182
      "",                          // 183
      "",                          // 184
      "",                          // 185
      "",                          // 186
      "",                          // 187
      "",                          // 188
      "",                          // 189
      "",                          // 190
      "",                          // 191
      "TW MMC3+VRAM Rev. B",       // 192
      "NTDEC TC-112",              // 193
      "TW MMC3+VRAM Rev. C",       // 194
      "TW MMC3+VRAM Rev. D",       // 195
      "",                          // 196
      "",                          // 197
      "TW MMC3+VRAM Rev. E",       // 198
      "",                          // 199
      "",                          // 200
      "",                          // 201
      "",                          // 202
      "",                          // 203
      "",                          // 204
      "",                          // 205
      "NAMCOT 108 Rev. C",         // 206
      "TAITO X1-005 Rev. B",       // 207
      "",                          // 208
      "",                          // 209
      "",                          // 210
      "",                          // 211
      "",                          // 212
      "",                          // 213
      "",                          // 214
      "",                          // 215
      "",                          // 216
      "",                          // 217
      "",                          // 218
      "UNLA9746",                  // 219
      "Debug Mapper",              // 220
      "UNLN625092",                // 221
      "",                          // 222
      "",                          // 223
      "",                          // 224
      "",                          // 225
      "BMC 22+20-in-1",            // 226
      "",                          // 227
      "",                          // 228
      "",                          // 229
      "BMC Contra+22-in-1",        // 230
      "",                          // 231
      "BMC QUATTRO",               // 232
      "BMC 22+20-in-1 RST",        // 233
      "BMC MAXI",                  // 234
      "",                          // 235
      "",                          // 236
      "",                          // 237
      "UNL6035052",                // 238
      "",                          // 239
      "",                          // 240
      "",                          // 241
      "",                          // 242
      "S74LS374NA",                // 243
      "DECATHLON",                 // 244
      "",                          // 245
      "FONG SHEN BANG",            // 246
      "",                          // 247
      "",                          // 248
      "",                          // 249
      "",                          // 250
      "",                          // 251
      "SAN GUO ZHI PIRATE",        // 252
      "DRAGON BALL PIRATE",        // 253
      "",                          // 254
      "",                          // 255
  };
  return strings[mapper];
}

enum Mirroring
{
  Mirroring_Horizontal = 0,
  Mirroring_Vertical = 1,
  Mirroring_FourScreen
};

static inline const char *string_from_mirroring(enum Mirroring mirroring)
{
  static const char *strings[] = {"Horizontal", "Vertical", "Four-screen"};
  return strings[mirroring];
}

enum Tv
{
  Tv_NTSC,
  Tv_PAL,
  Tv_Dual
};

static inline const char *string_from_tv(enum Tv tv)
{
  static const char *strings[] = {"NTSC", "PAL", "Dual"};
  return strings[tv];
}

enum RomFormat
{
  RomFormat_Unknown,
  RomFormat_iNes,
  RomFormat_Nes20
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
   * +------ 0: Board has no bus conflicts; 1: Board has bus conflicts
   */
  uint8_t flags_10;

  uint8_t padding[5];
};

/* iNES header information. */
struct iNesHeader
{
  uint8_t prg_rom_size;  // size of PRG ROM (program memory) in KB
  uint8_t chr_rom_size;  // size of CHR ROM (character data, graphics) in KB

  enum Mirroring mirroring;
  bool has_battery_backed_vram;
  bool has_trainer;
  enum Mapper mapper;
};

/* NES 2.0 header information. */
struct Nes2Header
{
  int prg_rom_size;  // size of PRG ROM (program memory) in 16KB units
  int chr_rom_size;  // size of CHR ROM (character data, graphics) in 8KB units
};

struct iNesHeader make_ines_header(uint8_t header_data[16]);
void ines_header_prg_data(struct iNesHeader header, uint8_t *rom_data,
                          uint8_t **prg_data);

enum RomFormat get_rom_format(uint8_t rom_header[16]);
void write_rom_information(FILE *fp, uint8_t *rom_data, size_t rom_size);

#endif
