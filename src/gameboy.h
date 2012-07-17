/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __GAMEBOY_H__
#define __GAMEBOY_H__

#include "cpu.h"
#include "cartridge.h"
#include "memory.h"

namespace Hardware
{
  class XGameboy
  {
    public:
      /*!
       *@brief Gameboy types.
       */
      enum EType
      {
        eGB,
        eGBC,
        eSGB,
        eGBP
      };

      XGameboy();
      ~XGameboy();

      void Reset();
      void Init();
      void Run(const std::string& file);
      void Pause();
      void Quit();

    private:
      void LoadCartridge(const std::string& file);

    public:
      //ACC/MUT
      bool GBType() { return (fType!=eGB?false:true); }
      void GBType(EType type) { fType=type; }

    private:
      EType fType;
      XCPU fCPU;
      XCartridge fCart;
      XMemory fMem;

      //Emulator related stuff
      bool fScreenHeight;
      bool fScreenWidth;
      bool fWithSound;
      bool fWithFPS;
      bool fIsRunning;
      bool fPause;
  };
}
#endif                           //__GAMEBOY_H__
