#ifndef NEPNES_ROM_H
#define NEPNES_ROM_H

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
  Mapper_ColorDreams = 11,            // ColorDreams chip (CrystalMines, TaginDragon, etc.)
  Mapper_F6XXX = 12,                  // FFE F6xxx (F6xxx games off FFE CDROM)
  Mapper_CPROM = 13,                  // CPROM switch
  Mapper_REX_SL1632 = 14,             // SL-1632 PCB (8-character version of Samurai Spirits)
  Mapper_100_IN_1 = 15,               // 100-in-1 switch
  Mapper_Bandai = 16,                 // Bandai chip (Japanese DragonBallZ series, etc.)
  Mapper_F8XXX = 17,                  // FFE F8xxx (F8xxx games off FFE CDROM)
  Mapper_Jaleco = 18,                 // Jaleco SS8806 chip (Japanese Baseball3, etc.)
  Mapper_Namco = 19,                  // Namco 106 chip (Japanese GhostHouse2, Baseball90, etc.)
  Mapper_VCR2_VCR4_REV_A = 21,        // Konami VCR2/VCR4 rev. a (Japanese WaiWaiWorld2, etc.)
  Mapper_VCR2_VCR4_REV_B = 22,        // Konami VCR2/VCR4 rev. b (Japanese TwinBee3)
  Mapper_VCR2_VCR4_REV_C = 23,        // Konami VCR2/VCR4 rev. c (Japanese WaiWaiWorld)
  Mapper_VCR6_REV_A = 24,             // Konami VCR6 rev. a
  Mapper_VCR2_VCR4_REV_D = 25,        // Konami VCR2/VCR4 rev. d
  Mapper_VCR6_REV_B = 26,             // Konami VCR6 rev. b
  Mapper_VCR4_PIRATE = 27,            // Believed to be a pirate variant of VCR4
  Mapper_Action53 = 28,               // Multicart discrete mapper
  Mapper_RetCuform = 29,              // RET-CUFROM
  Mapper_Unrom512 = 30,               // UNROM 512 (Study Hall, Mystic Origins)
  Mapper_Nsf = 31,                    //
  Mapper_IremG101 = 32,               // Irem G-101 chip (Japanese ImageFight, etc.)
  Mapper_TC0190FMC_TC0350FMR = 33,    //
  Mapper_IremBnRom = 34,              //
  Mapper_WarioLand2 = 35,             //
  Mapper_TxcPoliceman = 36,           //
  Mapper_PAL_ZZ_SMB_TETRIS_NWC = 37,  //
  Mapper_BitCorp = 38,                //
  Mapper_SMB2J_FDS = 40,              //
  Mapper_Caltron6In1 = 41,            //
  Mapper_BioMiracleFds = 42,          //
  Mapper_FDS_SMB2J_LF36 = 43,         //
  Mapper_MMC3_BMC_PIRATE_REV_A = 44,  //
  Mapper_MMC3_BMC_PIRATE_REV_B = 45,  //
  Mapper_RumbleStation15In1 = 46,     //
  Mapper_NES_QJ_SSVB_NWC = 48,        //
  Mapper_MMC3_BMC_REV_C = 49,         //
  Mapper_SMB2j_FDS_REV_A = 50,        //
  Mapper_11In1BallSeries = 51,        //
  Mapper_MMC3_BMC_PIRATE_REV_D = 52,  //
  Mapper_SUPERVISION_16_IN_1 = 53,    //
  Mapper_SIMPLE_BMC_REV_A = 57,       //
  Mapper_SIMPLE_BMC_REV_B = 58,       //
  Mapper_SIMPLE_BMC_REV_C = 60,       //
  Mapper_20In1KaiserRevA = 61,        //
  Mapper_700In1 = 62,                 //
  Mapper_Tengen = 64,                 //
  Mapper_IREM_H_3001 = 65,            //
  Mapper_MHROM = 66,                  //
  Mapper_SUNSOFT_FZII = 67,           //
  Mapper_SUNSOFT4 = 68,               //
  Mapper_SUNSOFT5_FME_7 = 69,         //
  Mapper_BaKamenDiscrete = 70,        //
  Mapper_Camerica = 71,               // Camerica chip
  Mapper_JALECO_JF_17 = 72,           //
  Mapper_KONAMI_VCR3 = 73,            //
  Mapper_TW_MMC3_VRAM_REV_A = 74,     //
  Mapper_KONAMI_VCR1 = 75,            //
  Mapper_NAMCOT_108_REV_A = 76,       //
  Mapper_IREM_LROG017 = 77,           //
  Mapper_IREM = 78,                   // Irem 74HC161/32-based
  Mapper_Nina3 = 79,                  // AVE Nina-3 board (KrazyKreatures, DoubleStrike, etc.)
  Mapper_TAITO_X1_005 = 80,           //
  Mapper_Nina6 = 81,                  // AVE Nina-6 board (Deathbots, MermaidsOfAtlantis, etc.)
  Mapper_TAITO_X1_017 = 82,           //
  Mapper_YOKO_VCR_REV_B = 83,         //
  Mapper_KONAMI_VCR7 = 85,            //
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
  Mapper_BandaiOekakids = 96,         //
  Mapper_IREM_TAM_S1 = 97,            //
  Mapper_VS_UNI_DUAL_SYSTEM = 99,     //
  Mapper_FDS_DOKIDOKI_FULL = 103,     //
  Mapper_NES_EVENT_NWC1990 = 105,     //
  Mapper_SMB3_PIRATE_A = 106,         //
  Mapper_MagicCorpA = 107,            //
  Mapper_FdsUnrom = 108,              //
  Mapper_Cheapocabra = 111,           //
  Mapper_AsderNtdec = 112,            //
  Mapper_HackerSachen = 113,          //
  Mapper_MMC3_SG_PROT_A = 114,        //
  Mapper_MMC3_PIRATE_A = 115,         //
  Mapper_MMC1_MMC3_VCR_PIRATE = 116,  //
  Mapper_FutureMedia = 117,           //
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
  Mapper_BandaiSram = 153,            //
  Mapper_BandaiBarcode = 157,         //
  Mapper_Bandai24C01 = 159,           //
  Mapper_SA009 = 160,                 //
  Mapper_SuborRevA = 166,             //
  Mapper_SuborRevB = 167,             //
  Mapper_BMCFK23C = 176,              //
  Mapper_TW_MMC3_VRAM_REV_B = 192,    //
  Mapper_NTDEC_TC_112 = 193,          //
  Mapper_TW_MMC3_VRAM_REV_C = 194,    //
  Mapper_TW_MMC3_VRAM_REV_D = 195,    //
  Mapper_TW_MMC3_VRAM_REV_E = 198,    //
  Mapper_NAMCOT_108_REV_C = 206,      //
  Mapper_TAITO_X1_005_REV_B = 207,    //
  Mapper_UNLA9746 = 219,              //
  Mapper_DebugMapper = 220,           //
  Mapper_UNLN625092 = 221,            //
  Mapper_BMC_22_20_IN_1 = 226,        //
  Mapper_BMC_Contra_22_IN_1 = 230,    //
  Mapper_BMC_QUATTRO = 232,           //
  Mapper_BMC_22_20_IN_1_RST = 233,    //
  Mapper_BmcMaxi = 234,               //
  Mapper_UNL6035052 = 238,            //
  Mapper_S74LS374NA = 243,            //
  Mapper_Decathlon = 244,             //
  Mapper_FongShenBang = 246,          //
  Mapper_SanGuiZhiPirate = 252,       //
  Mapper_DragonBallPirate = 253,      //
};
/* clang-format on */

static inline const char *mapper_to_string(enum Mapper mapper)
{
  static const char *strings[] = {
      [Mapper_NROM] = "NROM",
      [Mapper_MMC1] = "MMC1",
      [Mapper_UXROM] = "UxROM",
      [Mapper_CNROM] = "CNROM",
      [Mapper_MMC3] = "MMC3",
      [Mapper_MMC5] = "MMC5",
      [Mapper_F4XXX] = "FFE Rev. A",
      [Mapper_AxROM] = "AxROM",
      [Mapper_F3XXX] = "",
      [Mapper_MMC2] = "MMC2",
      [Mapper_MMC4I] = "MMC4",
      [Mapper_ColorDreams] = "Color Dreams",
      "MMC3 Rev. A",
      [Mapper_CPROM] = "CPROM",
      [Mapper_REX_SL1632] = "Rex SL-1632",
      [Mapper_100_IN_1] = "100-in-1",
      [Mapper_Bandai] = "BANDAI",
      [Mapper_F8XXX] = "FFE Rev. B",
      [Mapper_Jaleco] = "Jaleco SS88006",
      [Mapper_Namco] = "Namco 129/163",
      [Mapper_VCR2_VCR4_REV_A] = "Konami VCR2/VCR4 rev. a",
      [Mapper_VCR2_VCR4_REV_B] = "Konami VCR2/VCR4 rev. b",
      [Mapper_VCR2_VCR4_REV_C] = "Konami VCR2/VCR4 rev. c",
      [Mapper_VCR6_REV_A] = "Konami VCR6 rev. a",
      [Mapper_VCR2_VCR4_REV_D] = "Konami VCR2/VCR4 rev. d",
      [Mapper_VCR6_REV_B] = "Konami VCR6 rev. b",
      [Mapper_VCR4_PIRATE] = "Pirated VCR4",
      [Mapper_Action53] = "Action 53",
      [Mapper_RetCuform] = "RET-CUFORM",
      [Mapper_Unrom512] = "UNROM 512",
      [Mapper_Nsf] = "NSF",
      [Mapper_IremG101] = "IREM G-101",
      [Mapper_TC0190FMC_TC0350FMR] = "TC0190FMC/TC0350FMR",
      [Mapper_IremBnRom] = "IREM I-IM/BNROM",
      [Mapper_WarioLand2] = "Wario Land 2",
      [Mapper_TxcPoliceman] = "TXC Policeman",
      [Mapper_PAL_ZZ_SMB_TETRIS_NWC] = "PAL-ZZ SMB/TETRIS/NWC",
      [Mapper_BitCorp] = "Bit Corp.",
      [Mapper_SMB2J_FDS] = "SMB2j FDS",
      [Mapper_Caltron6In1] = "CALTRON 6-in-1",
      [Mapper_BioMiracleFds] = "BIO MIRACLE FDS",
      [Mapper_FDS_SMB2J_LF36] = "FDS SMB2j LF36",
      [Mapper_MMC3_BMC_PIRATE_REV_A] = "MMC3 BMC PIRATE rev. a",
      [Mapper_MMC3_BMC_PIRATE_REV_B] = "MMC3 BMC PIRATE rev. b",
      [Mapper_RumbleStation15In1] = "RUMBLESTATION 15-in-1",
      [Mapper_NES_QJ_SSVB_NWC] = "NES-QJ SSVB/NWC",
      [Mapper_MMC3_BMC_REV_C] = "MMC3 BMC PIRATE rev. c",
      [Mapper_SMB2j_FDS_REV_A] = "SMB2j FDS rev. A",
      [Mapper_11In1BallSeries] = "11-in-1 BALL SERIES",
      [Mapper_MMC3_BMC_PIRATE_REV_D] = "MMC3 BMC PIRATE rev. d",
      [Mapper_SUPERVISION_16_IN_1] = "SUPERVISION 16-in-1",
      [Mapper_SIMPLE_BMC_REV_A] = "SIMPLE BMC PIRATE rev. a",
      [Mapper_SIMPLE_BMC_REV_B] = "SIMPLE BMC PIRATE rev. b",
      [Mapper_SIMPLE_BMC_REV_C] = "SIMPLE BMC PIRATE rev. c",
      [Mapper_20In1KaiserRevA] = "20-in-1 KAISER rev. a",
      [Mapper_700In1] = "700-in-1",
      [Mapper_Tengen] = "TENGEN RAMBO1",
      [Mapper_IREM_H_3001] = "IREM H-3001",
      [Mapper_MHROM] = "MHROM",
      [Mapper_SUNSOFT_FZII] = "SUNSOFT-FZII",
      [Mapper_SUNSOFT4] = "SunSoft Mapper #4",
      [Mapper_SUNSOFT5_FME_7] = "SUNSOFT-5/FME-7",
      [Mapper_BaKamenDiscrete] = "BA KAMEN DISCRETE",
      [Mapper_Camerica] = "CAMERICA BF9093",
      [Mapper_JALECO_JF_17] = "JALECO JF-17",
      [Mapper_KONAMI_VCR3] = "KONAMI VCR3",
      [Mapper_TW_MMC3_VRAM_REV_A] = "TW MMC3+VRAM Rev. A",
      [Mapper_KONAMI_VCR1] = "KONAMI VCR1",
      [Mapper_NAMCOT_108_REV_A] = "NAMCOT 108 Rev. A",
      [Mapper_IREM_LROG017] = "IREM LROG017",
      [Mapper_IREM] = "Irem 74HC161/32",
      [Mapper_Nina3] = "AVE Nina 3/C&E/TXC board",
      [Mapper_Nina6] = "AVE Nina 6& board",
      [Mapper_TAITO_X1_005] = "TAITO X1-005 Rev. A",
      [Mapper_TAITO_X1_017] = "TAITO X1-017",
      [Mapper_YOKO_VCR_REV_B] = "YOKO VCR Rev. B",
      [Mapper_KONAMI_VCR7] = "KONAMI VCR7",
      [Mapper_JALECO_JF_13] = "JALECO JF-13",
      [Mapper_74x139_74_DISCRETE] = "74*139/74 DISCRETE",
      [Mapper_NAMCO_3433] = "NAMCO 3433",
      [Mapper_SUN_SOFT_3] = "SUNSOFT-3",
      [Mapper_HUMMER_JY] = "HUMMER/JY BOARD",
      [Mapper_EARLY_HUMMER_JY] = "EARLY HUMMER/JY BOARD",
      [Mapper_JALECO_JF_19] = "JALECO JF-19",
      [Mapper_SUN_SOFT_3R] = "SUNSOFT-3R",
      [Mapper_HVC_UN1ROM] = "HVC-UN1ROM",
      [Mapper_NAMCO_108_REV_B] = "NAMCOT 108 Rev. B",
      [Mapper_BandaiOekakids] = "BANDAI OEKAKIDS",
      [Mapper_IREM_TAM_S1] = "IREM TAM-S1",
      [Mapper_VS_UNI_DUAL_SYSTEM] = "VS Uni/Dual- system",
      [Mapper_FDS_DOKIDOKI_FULL] = "FDS DOKIDOKI FULL",
      [Mapper_NES_EVENT_NWC1990] = "NES-EVENT NWC1990",
      [Mapper_SMB3_PIRATE_A] = "SMB3 PIRATE A",
      [Mapper_MagicCorpA] = "Magic Corp A",
      [Mapper_FdsUnrom] = "Fds Unrom board",
      [Mapper_Cheapocabra] = "Cheapocabra",
      [Mapper_AsderNtdec] = "Asder/Ntdec board",
      [Mapper_HackerSachen] = "Hacker/Sachen board",
      [Mapper_MMC3_SG_PROT_A] = "MMC3 SG PROT. A",
      [Mapper_MMC3_PIRATE_A] = "MMC3 PIRATE A",
      [Mapper_MMC1_MMC3_VCR_PIRATE] = "MMC1/MMC3/VCR PIRATE",
      [Mapper_FutureMedia] = "Future Media board",
      [Mapper_TSK] = "TSKROM",
      [Mapper_TQROM] = "NES-TQROM",
      [Mapper_FDS_TOBIDASE] = "FDS TOBIDASE",
      [Mapper_MMC3_PIRATE_PROT_A] = "MMC3 PIRATE PROT. A",
      [Mapper_MMC3_PIRATE_H2288] = "MMC3 PIRATE H2288",
      [Mapper_FDS_LH32] = "FDS LH32",
      [Mapper_TXC_MGENIUS_22111] = "TXC/MGENIUS 22111",
      [Mapper_SA72008] = "SA72008",
      [Mapper_MMC3_BMC_PIRATE] = "MMC3 BMC PIRATE",
      [Mapper_TCU02] = "TCU02",
      [Mapper_S8259D] = "S8259D",
      [Mapper_S8259B] = "S8259B",
      [Mapper_S8259C] = "S8259C",
      [Mapper_JALECO_JF_11_14] = "JALECO JF-11/14",
      [Mapper_S8259A] = "S8259A",
      [Mapper_UNLKS7032] = "UNLKS7032",
      [Mapper_TCA01] = "TCA01",
      [Mapper_AGCI_50282] = "AGCI 50282",
      [Mapper_SA72007] = "SA72007",
      [Mapper_SA0161M] = "SA0161M",
      [Mapper_TCU01] = "TCU01",
      [Mapper_SA0037] = "SA0037",
      [Mapper_SA0036] = "SA0036",
      [Mapper_S74LS374N] = "S74LS374N",
      [Mapper_BandaiSram] = "BANDAI SRAM",
      [Mapper_BandaiBarcode] = "BANDAI BARCODE",
      [Mapper_Bandai24C01] = "BANDAI 24C01",
      [Mapper_SA009] = "SA009",
      [Mapper_SuborRevA] = "SUBOR Rev. A",
      [Mapper_SuborRevB] = "SUBOR Rev. B",
      [Mapper_BMCFK23C] = "BMCFK23C",
      [Mapper_TW_MMC3_VRAM_REV_B] = "TW MMC3+VRAM Rev. B",
      [Mapper_NTDEC_TC_112] = "NTDEC TC-112",
      [Mapper_TW_MMC3_VRAM_REV_C] = "TW MMC3+VRAM Rev. C",
      [Mapper_TW_MMC3_VRAM_REV_D] = "TW MMC3+VRAM Rev. D",
      [Mapper_TW_MMC3_VRAM_REV_E] = "TW MMC3+VRAM Rev. E",
      [Mapper_NAMCOT_108_REV_C] = "NAMCOT 108 Rev. C",
      [Mapper_TAITO_X1_005_REV_B] = "TAITO X1-005 Rev. B",
      [Mapper_UNLA9746] = "UNLA9746",
      [Mapper_DebugMapper] = "Debug Mapper",
      [Mapper_UNLN625092] = "UNLN625092",
      [Mapper_BMC_22_20_IN_1] = "BMC 22+20-in-1",
      [Mapper_BMC_Contra_22_IN_1] = "BMC Contra+22-in-1",
      [Mapper_BMC_QUATTRO] = "BMC QUATTRO",
      [Mapper_BMC_22_20_IN_1_RST] = "BMC 22+20-in-1 RST",
      [Mapper_BmcMaxi] = "BMC MAXI",
      [Mapper_UNL6035052] = "UNL6035052",
      [Mapper_S74LS374NA] = "S74LS374NA",
      [Mapper_Decathlon] = "Decathlon",
      [Mapper_FongShenBang] = "Fong Shen Bang",
      [Mapper_SanGuiZhiPirate] = "San Guo Zhi Pirate",
      [Mapper_DragonBallPirate] = "Dragon Ball Pirate",
  };
  return strings[mapper];
}

/*
 * TODO
 */
enum Mirroring
{
  Mirroring_Horizontal = 0,
  Mirroring_Vertical = 1,
  Mirroring_FourScreen
};

static inline const char *mirroring_to_string(enum Mirroring mirroring)
{
  static const char *strings[] = {"Horizontal", "Vertical", "Four-screen"};
  return strings[mirroring];
}

/*
 * CPU/PPU timing, indicates in which region the game was released.
 */
enum Tv
{
  Tv_Ntsc,
  Tv_Pal,
  Tv_Dual
};

/* Converts a TV enumeration value to string. */
static inline const char *tv_to_string(enum Tv tv)
{
  static const char *strings[] = {
      [Tv_Ntsc] = "NTSC", [Tv_Pal] = "PAL", [Tv_Dual] = "Dual"};
  return strings[tv];
}

/*
 * Enumeration of all console types that can be encoded in an iNes header.
 */
enum ConsoleType
{
  ConsoleType_NesFamicom = 0x0,
  ConsoleType_VsSystem = 0x1,
  ConsoleType_Playchoice10 = 0x2,
  ConsoleType_BitCorporationCreator = 0x3,
  ConsoleType_Vt01Monochrome = 0x4,
  ConsoleType_Vt01RedCyan = 0x5,
  ConsoleType_Vt02 = 0x6,
  ConsoleType_Vt03 = 0x7,
  ConsoleType_Vt09 = 0x8,
  ConsoleType_Vt32 = 0x9,
  ConsoleType_Vt369 = 0xa,
  ConsoleType_Um6578 = 0xb
};

/* Converts a console type enumeration value to string. */
static inline const char *console_type_to_string(enum ConsoleType console_type)
{
  static const char *strings[] = {
      [ConsoleType_NesFamicom] = "NES / Famicom / Dendy",
      [ConsoleType_VsSystem] = "VS System",
      [ConsoleType_Playchoice10] = "Playchoice-10",
      [ConsoleType_BitCorporationCreator] = "Bit Corporation Creator",
      [ConsoleType_Vt01Monochrome] = "VT01 Monochrome",
      [ConsoleType_Vt01RedCyan] = "VT01 Red/Cyan",
      [ConsoleType_Vt02] = "VT02",
      [ConsoleType_Vt03] = "VT03",
      [ConsoleType_Vt09] = "VT09",
      [ConsoleType_Vt32] = "VT32",
      [ConsoleType_Vt369] = "VT369",
      [ConsoleType_Um6578] = "UM6578"};
  return strings[console_type];
}

/*
 * Enumeration of supported ROM header formats. NES 2.0 is an extension of the
 * iNes ROM format.
 */
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
void rom_prg_data(struct RomHeader header, uint8_t *rom_data,
                  uint8_t **prg_data);
uint32_t rom_size_in_bytes(struct RomHeader *header);

enum RomFormat rom_get_format(uint8_t rom_header[16]);
int write_rom_information(FILE *fp, uint8_t *rom_data);

#endif  // NEPNES_ROM_H
