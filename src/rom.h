#ifndef NEPNES_ROM_H
#define NEPNES_ROM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* clang-format off */
enum Mapper
{
  MAPPER_NROM = 0,                    // All 32kB ROM + 8kB VROM games
  MAPPER_MMC1 = 1,                    // Nintendo MMC1 (Megaman2, Bomberman2, etc.)
  MAPPER_UXROM = 2,                   // UNROM/UOROM/etc. switch (QBert, PipeDream, Cybernoid,
                                      // many Japanese games)
  MAPPER_CNROM = 3,                   // CNROM switch (Castlevania, LifeForce, etc.)
  MAPPER_MMC3 = 4,                    // Nintendo MMC3 (SilverSurfer, SuperContra, Immortal, etc.)
  MAPPER_MMC5 = 5,                    // Nintendo MMC5 (Castlevania3)
  MAPPER_F4XXX = 6,                   // FFE F4xxx (F4xxx games off FFE CDROM)
  MAPPER_AXROM = 7,                   // AOROM switch (WizardsAndWarriors, Solstice, etc.)
  MAPPER_F3XXX = 8,                   // FFE F3xxx (F3xxx games off FFE CDROM)
  MAPPER_MMC2 = 9,                    // Nintendo MMC2 (Punchout)
  MAPPER_MMC4I = 10,                  // Nintendo MMC4 (Punchout2)
  MAPPER_COLOR_DREAMS = 11,           // ColorDreams chip (CrystalMines, TaginDragon, etc.)
  MAPPER_F6XXX = 12,                  // FFE F6xxx (F6xxx games off FFE CDROM)
  MAPPER_CPROM = 13,                  // CPROM switch
  MAPPER_REX_SL1632 = 14,             // SL-1632 PCB (8-character version of Samurai Spirits)
  MAPPER_100_IN_1 = 15,               // 100-in-1 switch
  MAPPER_BANDAI = 16,                 // Bandai chip (Japanese DragonBallZ series, etc.)
  MAPPER_F8XXX = 17,                  // FFE F8xxx (F8xxx games off FFE CDROM)
  MAPPER_JALECO = 18,                 // Jaleco SS8806 chip (Japanese Baseball3, etc.)
  MAPPER_NAMCO = 19,                  // Namco 106 chip (Japanese GhostHouse2, Baseball90, etc.)
  MAPPER_VCR2_VCR4_REV_A = 21,        // Konami VCR2/VCR4 rev. a (Japanese WaiWaiWorld2, etc.)
  MAPPER_VCR2_VCR4_REV_B = 22,        // Konami VCR2/VCR4 rev. b (Japanese TwinBee3)
  MAPPER_VCR2_VCR4_REV_C = 23,        // Konami VCR2/VCR4 rev. c (Japanese WaiWaiWorld)
  MAPPER_VCR6_REV_A = 24,             // Konami VCR6 rev. a
  MAPPER_VCR2_VCR4_REV_D = 25,        // Konami VCR2/VCR4 rev. d
  MAPPER_VCR6_REV_B = 26,             // Konami VCR6 rev. b
  MAPPER_VCR4_PIRATE = 27,            // Believed to be a pirate variant of VCR4
  MAPPER_ACTION_53 = 28,              // Multicart discrete mapper
  MAPPER_RET_CUFORM = 29,             // RET-CUFROM
  MAPPER_UNROM_512 = 30,              // UNROM 512 (Study Hall, Mystic Origins)
  MAPPER_NSF = 31,                    //
  MAPPER_IREM_G101 = 32,              // Irem G-101 chip (Japanese ImageFight, etc.)
  MAPPER_TC0190FMC_TC0350FMR = 33,    //
  MAPPER_IREM_BNROM = 34,             //
  MAPPER_WARIO_LAND_2 = 35,           //
  MAPPER_TXC_POLICEMAN = 36,          //
  MAPPER_PAL_ZZ_SMB_TETRIS_NWC = 37,  //
  MAPPER_BIT_CORP = 38,               //
  MAPPER_SMB2J_FDS = 40,              //
  MAPPER_CALTRON_6_IN_1 = 41,         //
  MAPPER_BIO_MIRACLE_FDS = 42,        //
  MAPPER_FDS_SMB2J_LF36 = 43,         //
  MAPPER_MMC3_BMC_PIRATE_REV_A = 44,  //
  MAPPER_MMC3_BMC_PIRATE_REV_B = 45,  //
  MAPPER_RUMBLE_STATION_15_IN_1 = 46, //
  MAPPER_NES_QJ_SSVB_NWC = 48,        //
  MAPPER_MMC3_BMC_REV_C = 49,         //
  MAPPER_SMB2j_FDS_REV_A = 50,        //
  MAPPER_11_IN_1_BALL_SERIES = 51,    //
  MAPPER_MMC3_BMC_PIRATE_REV_D = 52,  //
  MAPPER_SUPERVISION_16_IN_1 = 53,    //
  MAPPER_SIMPLE_BMC_REV_A = 57,       //
  MAPPER_SIMPLE_BMC_REV_B = 58,       //
  MAPPER_SIMPLE_BMC_REV_C = 60,       //
  MAPPER_20_IN_1_KAISER_REV_A = 61,   //
  MAPPER_700_IN_1 = 62,               //
  MAPPER_TENGEN = 64,                 //
  MAPPER_IREM_H_3001 = 65,            //
  MAPPER_MHROM = 66,                  //
  MAPPER_SUNSOFT_FZII = 67,           //
  MAPPER_SUNSOFT4 = 68,               //
  MAPPER_SUNSOFT5_FME_7 = 69,         //
  MAPPER_BA_KAMEN_DISCRETE = 70,      //
  MAPPER_CAMERICA = 71,               // Camerica chip
  MAPPER_JALECO_JF_17 = 72,           //
  MAPPER_KONAMI_VCR3 = 73,            //
  MAPPER_TW_MMC3_VRAM_REV_A = 74,     //
  MAPPER_KONAMI_VCR1 = 75,            //
  MAPPER_NAMCOT_108_REV_A = 76,       //
  MAPPER_IREM_LROG017 = 77,           //
  MAPPER_IREM = 78,                   // Irem 74HC161/32-based
  MAPPER_NINA_3 = 79,                 // AVE Nina-3 board (KrazyKreatures, DoubleStrike, etc.)
  MAPPER_TAITO_X1_005 = 80,           //
  MAPPER_NINA_6 = 81,                 // AVE Nina-6 board (Deathbots, MermaidsOfAtlantis, etc.)
  MAPPER_TAITO_X1_017 = 82,           //
  MAPPER_YOKO_VCR_REV_B = 83,         //
  MAPPER_KONAMI_VCR7 = 85,            //
  MAPPER_JALECO_JF_13 = 86,           //
  MAPPER_74x139_74_DISCRETE = 87,     //
  MAPPER_NAMCO_3433 = 88,             //
  MAPPER_SUN_SOFT_3 = 89,             //
  MAPPER_HUMMER_JY = 90,              //
  MAPPER_EARLY_HUMMER_JY = 91,        // Pirate HK-SF3 chip
  MAPPER_JALECO_JF_19 = 92,           //
  MAPPER_SUN_SOFT_3R = 93,            //
  MAPPER_HVC_UN1ROM = 94,             //
  MAPPER_NAMCO_108_REV_B = 95,        //
  MAPPER_BANDAI_OEKAKIDS = 96,        //
  MAPPER_IREM_TAM_S1 = 97,            //
  MAPPER_VS_UNI_DUAL_SYSTEM = 99,     //
  MAPPER_FDS_DOKIDOKI_FULL = 103,     //
  MAPPER_NES_EVENT_NWC1990 = 105,     //
  MAPPER_SMB3_PIRATE_A = 106,         //
  MAPPER_MAGIC_CORP_A = 107,          //
  MAPPER_FDS_UNROM = 108,             //
  MAPPER_CHEAPOCABRA = 111,           //
  MAPPER_ASDER_NTDEC = 112,           //
  MAPPER_HACKER_SACHEN = 113,         //
  MAPPER_MMC3_SG_PROT_A = 114,        //
  MAPPER_MMC3_PIRATE_A = 115,         //
  MAPPER_MMC1_MMC3_VCR_PIRATE = 116,  //
  MAPPER_FUTURE_MEDIA = 117,          //
  MAPPER_TSK = 118,                   //
  MAPPER_TQROM = 119,                 //
  MAPPER_FDS_TOBIDASE = 120,          //
  MAPPER_MMC3_PIRATE_PROT_A = 121,    //
  MAPPER_MMC3_PIRATE_H2288 = 123,     //
  MAPPER_FDS_LH32 = 125,              //
  MAPPER_TXC_MGENIUS_22111 = 132,     //
  MAPPER_SA72008 = 133,               //
  MAPPER_MMC3_BMC_PIRATE = 134,       //
  MAPPER_TCU02 = 136,                 //
  MAPPER_S8259D = 137,                //
  MAPPER_S8259B = 138,                //
  MAPPER_S8259C = 139,                //
  MAPPER_JALECO_JF_11_14 = 140,       //
  MAPPER_S8259A = 141,                //
  MAPPER_UNLKS7032 = 142,             //
  MAPPER_TCA01 = 143,                 //
  MAPPER_AGCI_50282 = 144,            //
  MAPPER_SA72007 = 145,               //
  MAPPER_SA0161M = 146,               //
  MAPPER_TCU01 = 147,                 //
  MAPPER_SA0037 = 148,                //
  MAPPER_SA0036 = 149,                //
  MAPPER_S74LS374N = 150,             //
  MAPPER_BANDAI_SRAM = 153,           //
  MAPPER_BANDAI_BARCODE = 157,        //
  MAPPER_BANDAI_24C01 = 159,          //
  MAPPER_SA009 = 160,                 //
  MAPPER_SUBOR_REV_A = 166,           //
  MAPPER_SUBOR_REV_B = 167,           //
  MAPPER_BMCFK23C = 176,              //
  MAPPER_TW_MMC3_VRAM_REV_B = 192,    //
  MAPPER_NTDEC_TC_112 = 193,          //
  MAPPER_TW_MMC3_VRAM_REV_C = 194,    //
  MAPPER_TW_MMC3_VRAM_REV_D = 195,    //
  MAPPER_TW_MMC3_VRAM_REV_E = 198,    //
  MAPPER_NAMCOT_108_REV_C = 206,      //
  MAPPER_TAITO_X1_005_REV_B = 207,    //
  MAPPER_UNLA9746 = 219,              //
  MAPPER_DEBUG_MAPPER = 220,          //
  MAPPER_UNLN625092 = 221,            //
  MAPPER_BMC_22_20_IN_1 = 226,        //
  MAPPER_BMC_CONTRA_22_IN_1 = 230,    //
  MAPPER_BMC_QUATTRO = 232,           //
  MAPPER_BMC_22_20_IN_1_RST = 233,    //
  MAPPER_BMC_MAXI = 234,              //
  MAPPER_UNL6035052 = 238,            //
  MAPPER_S74LS374NA = 243,            //
  MAPPER_DECATHLON = 244,             //
  MAPPER_FONG_SHEN_BANG = 246,        //
  MAPPER_SAN_GUI_ZHI_PIRATE = 252,    //
  MAPPER_DRAGON_BALL_PIRATE = 253,    //
};
/* clang-format on */

static inline const char *mapper_to_string(enum Mapper mapper)
{
  static const char *strings[] = {
      [MAPPER_NROM] = "NROM",
      [MAPPER_MMC1] = "MMC1",
      [MAPPER_UXROM] = "UxROM",
      [MAPPER_CNROM] = "CNROM",
      [MAPPER_MMC3] = "MMC3",
      [MAPPER_MMC5] = "MMC5",
      [MAPPER_F4XXX] = "FFE Rev. A",
      [MAPPER_AXROM] = "AxROM",
      [MAPPER_F3XXX] = "",
      [MAPPER_MMC2] = "MMC2",
      [MAPPER_MMC4I] = "MMC4",
      [MAPPER_COLOR_DREAMS] = "Color Dreams",
      "MMC3 Rev. A",
      [MAPPER_CPROM] = "CPROM",
      [MAPPER_REX_SL1632] = "Rex SL-1632",
      [MAPPER_100_IN_1] = "100-in-1",
      [MAPPER_BANDAI] = "BANDAI",
      [MAPPER_F8XXX] = "FFE Rev. B",
      [MAPPER_JALECO] = "Jaleco SS88006",
      [MAPPER_NAMCO] = "Namco 129/163",
      [MAPPER_VCR2_VCR4_REV_A] = "Konami VCR2/VCR4 rev. a",
      [MAPPER_VCR2_VCR4_REV_B] = "Konami VCR2/VCR4 rev. b",
      [MAPPER_VCR2_VCR4_REV_C] = "Konami VCR2/VCR4 rev. c",
      [MAPPER_VCR6_REV_A] = "Konami VCR6 rev. a",
      [MAPPER_VCR2_VCR4_REV_D] = "Konami VCR2/VCR4 rev. d",
      [MAPPER_VCR6_REV_B] = "Konami VCR6 rev. b",
      [MAPPER_VCR4_PIRATE] = "Pirated VCR4",
      [MAPPER_ACTION_53] = "Action 53",
      [MAPPER_RET_CUFORM] = "RET-CUFORM",
      [MAPPER_UNROM_512] = "UNROM 512",
      [MAPPER_NSF] = "NSF",
      [MAPPER_IREM_G101] = "IREM G-101",
      [MAPPER_TC0190FMC_TC0350FMR] = "TC0190FMC/TC0350FMR",
      [MAPPER_IREM_BNROM] = "IREM I-IM/BNROM",
      [MAPPER_WARIO_LAND_2] = "Wario Land 2",
      [MAPPER_TXC_POLICEMAN] = "TXC Policeman",
      [MAPPER_PAL_ZZ_SMB_TETRIS_NWC] = "PAL-ZZ SMB/TETRIS/NWC",
      [MAPPER_BIT_CORP] = "Bit Corp.",
      [MAPPER_SMB2J_FDS] = "SMB2j FDS",
      [MAPPER_CALTRON_6_IN_1] = "CALTRON 6-in-1",
      [MAPPER_BIO_MIRACLE_FDS] = "BIO MIRACLE FDS",
      [MAPPER_FDS_SMB2J_LF36] = "FDS SMB2j LF36",
      [MAPPER_MMC3_BMC_PIRATE_REV_A] = "MMC3 BMC PIRATE rev. a",
      [MAPPER_MMC3_BMC_PIRATE_REV_B] = "MMC3 BMC PIRATE rev. b",
      [MAPPER_RUMBLE_STATION_15_IN_1] = "RUMBLESTATION 15-in-1",
      [MAPPER_NES_QJ_SSVB_NWC] = "NES-QJ SSVB/NWC",
      [MAPPER_MMC3_BMC_REV_C] = "MMC3 BMC PIRATE rev. c",
      [MAPPER_SMB2j_FDS_REV_A] = "SMB2j FDS rev. A",
      [MAPPER_11_IN_1_BALL_SERIES] = "11-in-1 BALL SERIES",
      [MAPPER_MMC3_BMC_PIRATE_REV_D] = "MMC3 BMC PIRATE rev. d",
      [MAPPER_SUPERVISION_16_IN_1] = "SUPERVISION 16-in-1",
      [MAPPER_SIMPLE_BMC_REV_A] = "SIMPLE BMC PIRATE rev. a",
      [MAPPER_SIMPLE_BMC_REV_B] = "SIMPLE BMC PIRATE rev. b",
      [MAPPER_SIMPLE_BMC_REV_C] = "SIMPLE BMC PIRATE rev. c",
      [MAPPER_20_IN_1_KAISER_REV_A] = "20-in-1 KAISER rev. a",
      [MAPPER_700_IN_1] = "700-in-1",
      [MAPPER_TENGEN] = "TENGEN RAMBO1",
      [MAPPER_IREM_H_3001] = "IREM H-3001",
      [MAPPER_MHROM] = "MHROM",
      [MAPPER_SUNSOFT_FZII] = "SUNSOFT-FZII",
      [MAPPER_SUNSOFT4] = "SunSoft Mapper #4",
      [MAPPER_SUNSOFT5_FME_7] = "SUNSOFT-5/FME-7",
      [MAPPER_BA_KAMEN_DISCRETE] = "BA KAMEN DISCRETE",
      [MAPPER_CAMERICA] = "CAMERICA BF9093",
      [MAPPER_JALECO_JF_17] = "JALECO JF-17",
      [MAPPER_KONAMI_VCR3] = "KONAMI VCR3",
      [MAPPER_TW_MMC3_VRAM_REV_A] = "TW MMC3+VRAM Rev. A",
      [MAPPER_KONAMI_VCR1] = "KONAMI VCR1",
      [MAPPER_NAMCOT_108_REV_A] = "NAMCOT 108 Rev. A",
      [MAPPER_IREM_LROG017] = "IREM LROG017",
      [MAPPER_IREM] = "Irem 74HC161/32",
      [MAPPER_NINA_3] = "AVE Nina 3/C&E/TXC board",
      [MAPPER_NINA_6] = "AVE Nina 6& board",
      [MAPPER_TAITO_X1_005] = "TAITO X1-005 Rev. A",
      [MAPPER_TAITO_X1_017] = "TAITO X1-017",
      [MAPPER_YOKO_VCR_REV_B] = "YOKO VCR Rev. B",
      [MAPPER_KONAMI_VCR7] = "KONAMI VCR7",
      [MAPPER_JALECO_JF_13] = "JALECO JF-13",
      [MAPPER_74x139_74_DISCRETE] = "74*139/74 DISCRETE",
      [MAPPER_NAMCO_3433] = "NAMCO 3433",
      [MAPPER_SUN_SOFT_3] = "SUNSOFT-3",
      [MAPPER_HUMMER_JY] = "HUMMER/JY BOARD",
      [MAPPER_EARLY_HUMMER_JY] = "EARLY HUMMER/JY BOARD",
      [MAPPER_JALECO_JF_19] = "JALECO JF-19",
      [MAPPER_SUN_SOFT_3R] = "SUNSOFT-3R",
      [MAPPER_HVC_UN1ROM] = "HVC-UN1ROM",
      [MAPPER_NAMCO_108_REV_B] = "NAMCOT 108 Rev. B",
      [MAPPER_BANDAI_OEKAKIDS] = "BANDAI OEKAKIDS",
      [MAPPER_IREM_TAM_S1] = "IREM TAM-S1",
      [MAPPER_VS_UNI_DUAL_SYSTEM] = "VS Uni/Dual- system",
      [MAPPER_FDS_DOKIDOKI_FULL] = "FDS DOKIDOKI FULL",
      [MAPPER_NES_EVENT_NWC1990] = "NES-EVENT NWC1990",
      [MAPPER_SMB3_PIRATE_A] = "SMB3 PIRATE A",
      [MAPPER_MAGIC_CORP_A] = "Magic Corp A",
      [MAPPER_FDS_UNROM] = "Fds Unrom board",
      [MAPPER_CHEAPOCABRA] = "Cheapocabra",
      [MAPPER_ASDER_NTDEC] = "Asder/Ntdec board",
      [MAPPER_HACKER_SACHEN] = "Hacker/Sachen board",
      [MAPPER_MMC3_SG_PROT_A] = "MMC3 SG PROT. A",
      [MAPPER_MMC3_PIRATE_A] = "MMC3 PIRATE A",
      [MAPPER_MMC1_MMC3_VCR_PIRATE] = "MMC1/MMC3/VCR PIRATE",
      [MAPPER_FUTURE_MEDIA] = "Future Media board",
      [MAPPER_TSK] = "TSKROM",
      [MAPPER_TQROM] = "NES-TQROM",
      [MAPPER_FDS_TOBIDASE] = "FDS TOBIDASE",
      [MAPPER_MMC3_PIRATE_PROT_A] = "MMC3 PIRATE PROT. A",
      [MAPPER_MMC3_PIRATE_H2288] = "MMC3 PIRATE H2288",
      [MAPPER_FDS_LH32] = "FDS LH32",
      [MAPPER_TXC_MGENIUS_22111] = "TXC/MGENIUS 22111",
      [MAPPER_SA72008] = "SA72008",
      [MAPPER_MMC3_BMC_PIRATE] = "MMC3 BMC PIRATE",
      [MAPPER_TCU02] = "TCU02",
      [MAPPER_S8259D] = "S8259D",
      [MAPPER_S8259B] = "S8259B",
      [MAPPER_S8259C] = "S8259C",
      [MAPPER_JALECO_JF_11_14] = "JALECO JF-11/14",
      [MAPPER_S8259A] = "S8259A",
      [MAPPER_UNLKS7032] = "UNLKS7032",
      [MAPPER_TCA01] = "TCA01",
      [MAPPER_AGCI_50282] = "AGCI 50282",
      [MAPPER_SA72007] = "SA72007",
      [MAPPER_SA0161M] = "SA0161M",
      [MAPPER_TCU01] = "TCU01",
      [MAPPER_SA0037] = "SA0037",
      [MAPPER_SA0036] = "SA0036",
      [MAPPER_S74LS374N] = "S74LS374N",
      [MAPPER_BANDAI_SRAM] = "BANDAI SRAM",
      [MAPPER_BANDAI_BARCODE] = "BANDAI BARCODE",
      [MAPPER_BANDAI_24C01] = "BANDAI 24C01",
      [MAPPER_SA009] = "SA009",
      [MAPPER_SUBOR_REV_A] = "SUBOR Rev. A",
      [MAPPER_SUBOR_REV_B] = "SUBOR Rev. B",
      [MAPPER_BMCFK23C] = "BMCFK23C",
      [MAPPER_TW_MMC3_VRAM_REV_B] = "TW MMC3+VRAM Rev. B",
      [MAPPER_NTDEC_TC_112] = "NTDEC TC-112",
      [MAPPER_TW_MMC3_VRAM_REV_C] = "TW MMC3+VRAM Rev. C",
      [MAPPER_TW_MMC3_VRAM_REV_D] = "TW MMC3+VRAM Rev. D",
      [MAPPER_TW_MMC3_VRAM_REV_E] = "TW MMC3+VRAM Rev. E",
      [MAPPER_NAMCOT_108_REV_C] = "NAMCOT 108 Rev. C",
      [MAPPER_TAITO_X1_005_REV_B] = "TAITO X1-005 Rev. B",
      [MAPPER_UNLA9746] = "UNLA9746",
      [MAPPER_DEBUG_MAPPER] = "Debug Mapper",
      [MAPPER_UNLN625092] = "UNLN625092",
      [MAPPER_BMC_22_20_IN_1] = "BMC 22+20-in-1",
      [MAPPER_BMC_CONTRA_22_IN_1] = "BMC Contra+22-in-1",
      [MAPPER_BMC_QUATTRO] = "BMC QUATTRO",
      [MAPPER_BMC_22_20_IN_1_RST] = "BMC 22+20-in-1 RST",
      [MAPPER_BMC_MAXI] = "BMC MAXI",
      [MAPPER_UNL6035052] = "UNL6035052",
      [MAPPER_S74LS374NA] = "S74LS374NA",
      [MAPPER_DECATHLON] = "Decathlon",
      [MAPPER_FONG_SHEN_BANG] = "Fong Shen Bang",
      [MAPPER_SAN_GUI_ZHI_PIRATE] = "San Guo Zhi Pirate",
      [MAPPER_DRAGON_BALL_PIRATE] = "Dragon Ball Pirate",
  };
  return strings[mapper];
}

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

#endif  // NEPNES_ROM_H
