/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "ram.h"
#include <fstream>               //Save/Restore

namespace Hardware
{
  XRam::XRam()
  {
    Reset();
  }

  void XRam::Reset()
  {
    fBanksCount=0;
    fCurrentBank=0;
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
      fMem=0;
    }
  }

  XRam::~XRam()
  {
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }
  }

  void XRam::SetSize(unsigned char size)
  {
    switch(size)
    {
      case 0x0: fBanksCount=0;  break; //0 - None
      case 0x1: fBanksCount=1;  break; //1 - 16kBit  = 2kB  = 1 bank
      case 0x2: fBanksCount=1;  break; //2 - 64kBit  = 8kB  = 1 bank
      case 0x3: fBanksCount=4;  break; //3 - 256kBit = 32kB = 4 banks
      case 0x4: fBanksCount=16; break; //4 -   1MBit =128kB =16 banks
      default: fBanksCount=1; break;
    }

    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }

    fMem=new unsigned char[fBanksCount*RAM_BANK_SIZE];

    #if defined LOG
    std::cout << std::hex << "LOG - XRAM: Banks count : " << fBanksCount << '\n';
    std::cout << "LOG - XRAM: Allocate " << fBanksCount*RAM_BANK_SIZE << " @ " << (short*)fMem << '\n';
    #endif

    fCurrentBank=0;
  }

  //Save
  EErrors XRam::Save(const std::string& file)
  {
    std::fstream Game(file.c_str(), std::ios_base::out |std::ios_base::binary);
    if(!Game.is_open()) return eCannotSave;
    for(int i=0; i<fBanksCount*RAM_BANK_SIZE; ++i) Game.write((char*)fMem+i, 1);
    Game.close();
    return eNoError;
  }

  //Restore
  EErrors XRam::Load(const std::string& file)
  {
    std::fstream Game(file.c_str(), std::ios_base::in |std::ios_base::binary);
    if(!Game.is_open()) return eCannotLoad;
    for(int i=0; i<fBanksCount*RAM_BANK_SIZE; ++i) Game.read((char*)fMem+i, 1);
    Game.close();
    return eNoError;
  }

  unsigned char XRam::Read(unsigned short address)
  {
    fCurrentBank=address>>12;
    if(fCurrentBank>=0x0E) fCurrentBank-=2;

    unsigned short Add=0;
    if(address>=RAM_DUPLICATE) Add=address-RAM_DUPLICATE;
    else Add=address-RAM_START;
    return fMem[Add];
  }

  void XRam::Write(unsigned short address, unsigned char val)
  {
    unsigned short Add=0;
    if(address>=RAM_DUPLICATE) Add=address-RAM_DUPLICATE;
    else Add=address-RAM_START;
    fMem[Add]=val;
  }
}
