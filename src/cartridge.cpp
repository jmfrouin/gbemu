/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "cartridge.h"
#include "def.h"
#include <iostream>

namespace Hardware
{
  XCartridge::XCartridge():
  fMemory(0), fType(eUnknow), fRomSize(0), fRamSize(0)
  {

  }

  XCartridge::~XCartridge()
  {
    if(fMemory) { delete fMemory; fMemory=0; }
  }

  void XCartridge::Reset()
  {
    if(fMemory) delete fMemory;
    fMemory=new unsigned char[GB_CARTRIDGE_SIZE];
  }

  void XCartridge::Load(std::fstream* stream)
  {
    for(int i=0; i<GB_CARTRIDGE_SIZE; i++) stream->read((char*)fMemory+i, 1);
    fGameName=(const char*)(fMemory+0x134);
    #if defined LOG
    std::cout << "LOG - XCartridge: Game name: " << fGameName << "\n";
    #endif

    //Classic & color gameboys
    #if defined LOG
    if(fMemory[0x143]==0x80) std::cout << "LOG - XCartridge: Color&Normal Gameboy\n";
    else if(fMemory[0x143]==0xC0) std::cout << "LOG - XCartridge: Color Gameboy only\n";
    else std::cout << "LOG - XCartridge: Gameboy\n";

    //Super gameboy
    if(fMemory[0x146]) std::cout <<"LOG - XCartridge: Super Gameboy\n";
    #endif

    fType=Type(fMemory[0x147]);
    fRomSize=fMemory[0x148];
    fRamSize=fMemory[0x149];

    #if defined LOG
    std::cout << "LOG - XCartridge: " << (fMemory[0x14A]?"Non japanese\n":"Japanese\n");
    #endif
  }

  XCartridge::EType XCartridge::Type(unsigned char type)
  {
    switch(type)
    {
      case 0x00: return eROM_ONLY;
      case 0x01: return eROM_MBC1;
      case 0x02: return eROM_MBC1_RAM;
      case 0x03: return eROM_MBC1_RAM_BATT;
      case 0x05: return eROM_MBC2;
      case 0x06: return eROM_MBC2_BATTERY;
      case 0x08: return eROM_RAM;
      case 0x09: return eROM_RAM_BATTERY;
      case 0x0B: return eROM_MMM01;
      case 0x0C: return eROM_MMM01_SRAM;
      case 0x0D: return eROM_MMM01_SRAM_BATT;
      case 0x0F: return eROM_MBC3_TIMER_BATT;
      case 0x10: return eROM_MBC3_RAM_BATT;
      case 0x11: return eROM_MBC3;
      case 0x12: return eROM_MBC3_RAM;
      case 0x13: return eROM_MBC3_RAM_BATT;
      case 0x19: return eROM_MBC5;
      case 0x1A: return eROM_MBC5_RAM;
      case 0x1B: return eROM_MBC5_RAM_BATT;
      case 0x1C: return eROM_MBC5_RUMBLE;
      case 0x1D: return eROM_MBC5_RUMBLE_SRAM;
      case 0x1E: return eROM_MBC5_RUMBLE_SRAM_BATT;
      case 0x1F: return ePocket_Camera;
      case 0xFD: return eBandai_TAMA5;
      case 0xFE: return eHudson_HuC3;
      case 0xFF: return eHudson_HuC1;
      default: return eUnknow;
    }
  }
}
