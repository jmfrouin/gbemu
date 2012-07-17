/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "rom.h"
#include "def.h"
#include <iostream>

namespace Hardware
{
  XRom::XRom()
  {
  }

  void XRom::SetSize(unsigned char size)
  {
    switch(size)
    {
      default:
      case 0x0:                                   //0 - 256Kbit =  32KByte =   2 banks
      case 0x1:                                   //1 - 512Kbit =  64KByte =   4 banks
      case 0x2:                                   //2 -   1Mbit = 128KByte =   8 banks
      case 0x3:                                   //3 -   2Mbit = 256KByte = 16 banks
      case 0x4:                                   //4 -   4Mbit = 512KByte = 32 banks
      case 0x5:                                   //5 -   8Mbit =   1MByte = 64 banks
      case 0x6: fBanksCount=1<<size<<1; break;    //6 - 16Mbit  =   2MByte = 128 banks
      case 0x52: fBanksCount=72; break;         //$52 -   9Mbit = 1.1MByte = 72 banks
      case 0x53: fBanksCount=80; break;         //$53 - 10Mbit  = 1.2MByte = 80 banks
      case 0x54: fBanksCount=96; break;         //$54 - 12Mbit  = 1.5MByte = 96 banks
    }
    //rom_mask=(1<<gb_memory[0x148]<<1)-1;
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XROM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }
    fMem=new unsigned char[fBanksCount*ROM_BANK_SIZE];
    #if defined LOG
    std::cout << std::hex << "LOG - XROM: Banks count : " << fBanksCount << '\n';
    std::cout << "LOG - XROM: Allocate " << fBanksCount*ROM_BANK_SIZE << " @ " << (short*)fMem << '\n';
    #endif
    fCurrentBank=1;
  }

  void XRom::Reset()
  {
    fBanksCount=0;
    fCurrentBank=0;
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XROM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
      fMem=0;
    }
  }

  XRom::~XRom()
  {
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XROM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }
  }

  unsigned char XRom::Read(unsigned short address)
  {
    return fMem[address-ROM_START];
  }

  void XRom::Write(unsigned short address, unsigned char val)
  {
    fMem[address-ROM_START]=val;
  }
}
