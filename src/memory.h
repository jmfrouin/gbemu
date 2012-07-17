/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "switchableram.h"
#include "videoram.h"
#include "ram.h"
#include "rom.h"

#include "cartridge.h"

//Registers
#define P1_REG 0x00 //Joypad
#define SB_REG 0x01
#define SC_REG 0x02
#define DIV_REG 0x04
#define TIMA_COUNT_REG 0x05
#define TIMA_MOD_REG 0x06
#define TAC_REG 0x07
#define IF_REG 0x0F
#define NR10_REG 0x10
#define NR11_REG 0x11
#define NR12_REG 0x12
#define NR13_REG 0x13
#define NR14_REG 0x14
#define NR21_REG 0x16
#define NR22_REG 0x17
#define NR23_REG 0x18
#define NR24_REG 0x19
#define NR30_REG 0x1A
#define NR31_REG 0x1B
#define NR32_REG 0x1C
#define NR33_REG 0x1D
#define NR34_REG 0x1E
#define NR41_REG 0x20
#define NR42_REG 0x21
#define NR43_REG 0x22
#define NR44_REG 0x23
#define NR50_REG 0x24
#define NR51_REG 0x25
#define NR52_REG 0x26

#define LCDC_CONT_REG 0x40
#define LCDC_STAT_REG 0x41
#define SCY_REG 0x42
#define SCX_REG 0x43
#define LY_REG 0x44
#define LYC_REG 0x45
#define DMA_REG 0x46
#define BGP_REG 0x47
#define OBP0_REG 0x48
#define OBP1_REG 0x49
#define WY_REG 0x4A
#define WX_REG 0x4B

namespace Hardware
{
  class XMemory
  {
    public:
      XMemory();
      ~XMemory();
      void Test();

      /*!
       * @brief Reset all memories.
       * @author snoogie (11/18/2008)
       */
      void Reset(XCartridge::EType);

      /*!
       * @brief Read memory
       * @author snoogie (11/27/2008)
       * @param address The address of data to read.
       * @return unsigned char Read value
       */
      unsigned char Read(unsigned short address);

      /*!
       * @brief Read a word.
       * @author snoogie (1/1/2009)
       * @param address
       * @return unsigned short Read value
       */
      unsigned short ReadW(unsigned short address);

      /*!
       * @brief Write memory
       * @author snoogie (12/20/2008)
       * @param address The address of data to write
       * @param val The value to write
       */
      void Write(XCPU* cpu, unsigned short address, unsigned char val);

      /*!
       * @brief Set background palette.
       * @author snoogie (3/15/2009)
       * @param val New value.
       */
      void SetPalBkg(unsigned char val);

    private:
      friend class XGameboy;
      friend class XCPU;
      friend class XVideoRam;

      unsigned char fRAMMask;

      unsigned char fMBC5High;
      unsigned char fMBC5Low;

      unsigned char fMBC1Mode;
      unsigned char fMBC1Line;
      
      unsigned char fInterruptRegister;//FFFF
      unsigned char fInternalRam[0x7F];//FF80-FFFF
      unsigned char fUnused2[0x34];//FF4C-FF80
      unsigned char fIOPorts[0x4C];//FF00-FF4C
      unsigned char fUnused1[0x60];//FEA0-FF00
      unsigned char fOAMMem[0xA0];//Sprite Attrib Memory (OAM) : FE00-FEA0
      XRom fRom;
      XRam fRam;
      XSwitchableRam fSwitchableRam;
      XVideoRam fVideoRam;
  };
}
#endif                           //__MEMORY_H__
