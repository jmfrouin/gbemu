/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __RAM_H__
#define __RAM_H__

#include "mem.h"
#include "def.h"

namespace Hardware
{
  /*!
   *@brief Ram of GB
   *@note --------------------------- 0xFE00
   *@note Echo of 8kB Internal RAM
   *@note --------------------------- 0xE000
   *@note 8kB Internal RAM
   *@note --------------------------- 0xC000
   */
  class XRam : public IMem
  {
    public:
      ///Ctor/Dtor
      XRam();
      ~XRam();
      void Reset();

      //From IMen
      unsigned char Read(unsigned short address);
      void Write(unsigned short address, unsigned char val);

      /*!
       * @brief Set and allocate memory.
       * @param size The size type.
       */
      void SetSize(unsigned char size);

      /*!
       *@brief Backup Ram (A game)
       *@param file Location of file.
       *@return An error code from EErrors.
       */
      EErrors Save(const std::string& file);

      /*!
       *@brief Restore a backup of Ram (A game saved)
       *@param file Location of backup file.
       *@return An error code from EErrors.
       */
      EErrors Load(const std::string& file);
  };
}
#endif                           //__RAM_H__
