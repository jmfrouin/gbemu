/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __ROM_H__
#define __ROM_H__

#include "cartridge.h"
#include "mem.h"

namespace Hardware
{
  /* 
      --------------------------- 8000 --
      16kB switchable ROM bank          |
      --------------------------- 4000  |= 32kB Cartrigbe
       16kB ROM bank #0                 |
      --------------------------- 0000 --*/
  class XRom : public IMem
  {
    public:
      XRom();
      ~XRom();

      //From IMen
      unsigned char Read(unsigned short address);
      void Write(unsigned short address, unsigned char val);

      void SetSize(unsigned char size);
      void Reset();
  };
}
#endif                           //__ROM_H__
