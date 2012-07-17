/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include <string>
#include <fstream>

namespace Hardware
{
  class XCartridge
  {
    public:
      XCartridge();
      ~XCartridge();

      void Load(std::fstream* stream);
      void Reset();

      /*!
       *@brief Define at byte 0x147 of cartridge.
       */
      enum EType
      {
        eUnknow,
        eROM_ONLY,
        eROM_MBC1,
        eROM_MBC1_RAM,
        eROM_MBC1_RAM_BATT,
        eROM_MBC2,
        eROM_MBC2_BATTERY,
        eROM_MBC3_TIMER_BATT,
        eROM_MBC3_TIMER_RAM_BATT,
        eROM_MBC3,
        eROM_MBC3_RAM,
        eROM_MBC3_RAM_BATT,
        eROM_MBC5,
        eROM_MBC5_RAM,
        eROM_MBC5_RAM_BATT,
        eROM_MBC5_RUMBLE,
        eROM_MBC5_RUMBLE_SRAM,
        eROM_MBC5_RUMBLE_SRAM_BATT,
        eROM_MMM01,
        eROM_MMM01_SRAM,
        eROM_MMM01_SRAM_BATT,
        eROM_RAM,
        eROM_RAM_BATTERY,
        ePocket_Camera,
        eBandai_TAMA5,
        eHudson_HuC3,
        eHudson_HuC1
      };

    private:
      /*
       *@brief Defined by byte 0x147 from the cartridge.
       */
      EType Type(unsigned char type);
      unsigned char ROMSize(unsigned char type);
      unsigned char RAMSize(unsigned char type);

    private:
      friend class XGameboy;
      unsigned char* fMemory;
      EType fType;
      std::string fGameName;
      unsigned char fRomSize;
      unsigned char fRamSize;
  };
}
#endif                           //__CARTRIDGE_H__
