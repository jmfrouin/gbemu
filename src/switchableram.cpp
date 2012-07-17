/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "switchableram.h"
#include "def.h"

namespace Hardware
{
  XSwitchableRam::XSwitchableRam()
  {
    Reset();
  }

  XSwitchableRam::~XSwitchableRam()
  {
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XSwitchableRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }
  }

  void XSwitchableRam::Reset()
  {
    fBanksCount=0;
    fCurrentBank=0;
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XSwitchableRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
      fMem=0;
    }
  }

  void XSwitchableRam::SetSize()
  {
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XSwitchableRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }

    fMem=new unsigned char[fBanksCount*RAM_BANK_SIZE];
    #if defined LOG
    std::cout << std::hex << "LOG - XSwitchableRAM: Banks count : " << fBanksCount << '\n';
    std::cout << "LOG - XSwitchableRAM: Allocate " << fBanksCount*(RAM_BANK_SIZE>>1) << " @ " << (short*)fMem << '\n';
    #endif
    fCurrentBank=1;
  }

  unsigned char XSwitchableRam::Read(unsigned short address)
  {
    return fMem[address-SWITCHABLE_RAM_START];
  }

  void XSwitchableRam::Write(unsigned short address, unsigned char val)
  {
    fMem[address-SWITCHABLE_RAM_START]=val;
  }
}
