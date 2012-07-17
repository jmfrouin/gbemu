/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __MEM_H__
#define __MEM_H__

#include <iostream>

namespace Hardware
{
  class IMem
  {
    public:
      IMem():
      fBanksCount(0), fCurrentBank(0), fMem(0)
      {
      }

      virtual ~IMem()
      {
      }

      virtual void Reset() = 0;

      /*!
       * @brief Read memory
       * @author snoogie (12/20/2008)
       * @param address The address of data to read.
       * @return unsigned char Read value
       */
      virtual unsigned char Read(unsigned short address)=0;

      /*!
       * @brief Write memory
       * @author snoogie (12/20/2008)
       * @param address The address of data to write
       * @param val The value to write
       */
      virtual void Write(unsigned short address, unsigned char val)=0;

      //virtual void SetSize();

    protected:
      friend class XGameboy;
      unsigned short fBanksCount;
      unsigned short fCurrentBank;
      unsigned char* fMem;
  };
}
#endif                           //__MEM_H__
