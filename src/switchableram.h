/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __SWITCHABLERAM_H__
#define __SWITCHABLERAM_H__

#include "mem.h"

namespace Hardware
{
  /*
  --------------------------- 0xC000
   8kB switchable RAM bank
  --------------------------- 0xA000
  */
  class XSwitchableRam : public IMem
  {
    public:
      XSwitchableRam();
      ~XSwitchableRam();

      //From IMen
      unsigned char Read(unsigned short address);
      void Write(unsigned short address, unsigned char val);

      void Reset();
      void SetSize();
  };
}
#endif                           //__SWITCHABLERAM_H__
