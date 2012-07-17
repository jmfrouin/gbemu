/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "memory.h"
#include <memory.h>

namespace Hardware { class XGameboy; }

extern bool gTimer;
extern int gTicksCount;

namespace Hardware
{
  XMemory::XMemory()
  {
  }

  void XMemory::Reset(XCartridge::EType type)
  {
    fRAMMask=fMBC5High=fMBC5Low=fMBC1Mode=fMBC1Line=0;
    fInterruptRegister=0; //0xDE;
    memset(fInternalRam,0,0x7F);
    memset(fUnused2,0,0x34);
    //memset(fIOPorts,0,0x4C);
    unsigned char IOPorts[256]=
    {
      0xCF, 0x00, 0x7E, 0xFF, 0xAD, 0x00, 0x00, 0xF8, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x80, 0xBF, 0xF0, 0xFF,
      0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF,
      0xBF, 0xFF, 0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xFE,
      0x0E, 0x7F, 0x00, 0xFF, 0x58, 0xDF, 0x00, 0xEC, 0x00, 0xBF,
      0x0c, 0xED, 0x03, 0xF7, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
      0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xC1, 0x20, 0x00, 0x00,
      0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    for(int i=0; i<0x4C; i++) fIOPorts[i]=IOPorts[i];

    fIOPorts[LY_REG]=0x3E;

    memset(fUnused1,0,0x60);
    memset(fOAMMem,0,0xA0);
    fRom.Reset();
    fRam.Reset();
    fVideoRam.Reset();
  }

  void XMemory::Test()
  {
    for(int i=0x4C;--i>=0;) fIOPorts[i]=i;
    unsigned char Ret=Read(0xFF05);
    std::cout << std::hex << Ret  << '\n';
    Ret=Read(0xFFFF);
    std::cout << std::hex << Ret  << '\n';
    Ret=Read(0xFF07);
    std::cout << std::hex << Ret  << '\n';
    Ret=Read(0xFF0F);
    std::cout << std::hex << Ret  << '\n';
    Ret=Read(0xFF41);
    std::cout << std::hex << Ret  << '\n';
    //Write(0xFF00, 0xEA);
  }

  XMemory::~XMemory()
  {
  }

  unsigned char XMemory::Read(unsigned short address)
  {
    unsigned char Ret=0xFF;
    switch(address)
    {
      case ROM_START ... ROM_END:
        Ret=fRom.Read(address);
        break;
      case VIDEO_RAM_START ... VIDEO_RAM_END:
        Ret=fVideoRam.Read(address);
        break;
      case SWITCHABLE_RAM_START ... SWITCHABLE_RAM_END:    //Switchable ram
        Ret=fSwitchableRam.Read(address);
        break;
      case RAM_START ... RAM_END:    //Internal ram
        Ret=fRam.Read(address);
        break;
      case 0xFE00 ... 0xFE9F:    //OAM
        Ret=fOAMMem[address&0xFF];
        break;
      case 0xFEA0 ... 0xFEFF:    //Empty but unusable for I/O
        break;
        //I/O Ports
      case 0xFF00 ... 0xFF4B:
      {
        int Port=address&0xFF;
        switch(Port)
        {
          case P1_REG: //Joyad
            {
              unsigned char Pad=fIOPorts[P1_REG];
              switch(Pad&0x30)
              {
                case 0x00:
                case 0xFF: Ret=0xFF; break;
                case 0x10: Ret=(~(Pad&0x0F))|0x10; break;
                case 0x20: Ret=(~(Pad>>4))|0x20; break;
                case 0x30: Ret=0xFF; break;
              }
            }
            break;
          case 0x01 ... 0x06:
            Ret=fIOPorts[Port];
            break;
          case TAC_REG://Bits 0,1,2 used
            Ret=fIOPorts[Port]|0xF8;
            break;
          case IF_REG://Bits 5,6,7 unused
            Ret=fIOPorts[Port]|0xE0;
            break;
          case 0x10 ... 0x3F:    //Sound
            break;               //Unsupported yet!!
          case LCDC_CONT_REG:
            Ret=fIOPorts[Port];
            break;
          case LCDC_STAT_REG: //Bit 7 unused
            Ret=fIOPorts[Port]|0x80;
            break;
          case SCY_REG:
          case SCX_REG:
          case LY_REG:
          case LYC_REG:
          case DMA_REG:
            Ret=fIOPorts[Port];
            break;
          case BGP_REG:
            Ret=fVideoRam.fBkgPalData[0]|(fVideoRam.fBkgPalData[1]<<2)|(fVideoRam.fBkgPalData[2]<<4)|(fVideoRam.fBkgPalData[3]<<6);
            break;
          case OBP0_REG:
            Ret=fVideoRam.fObjPal0Data[0]|(fVideoRam.fObjPal0Data[1]<<2)|(fVideoRam.fObjPal0Data[2]<<4)|(fVideoRam.fObjPal0Data[3]<<6);
            break;
          case OBP1_REG:
            Ret=fVideoRam.fObjPal1Data[0]|(fVideoRam.fObjPal1Data[1]<<2)|(fVideoRam.fObjPal1Data[2]<<4)|(fVideoRam.fObjPal1Data[3]<<6);
            break;
          case WY_REG:
          case WX_REG:
            Ret=fIOPorts[Port];
            break;
        }
        break;
      }
      case 0xFF4C ... 0xFF7F:    //Empty but unusable for I/O
        break;
      case 0xFF80 ... 0xFFFE:    //InternalRam
        Ret=fInternalRam[address-0xFF80];
        #if defined MEMORY_RW_DEBUG
        std::cout << "Memory read @ 0x" << std::hex << address << " = 0x" << (short)Ret << std::dec << '\n';
        #endif
        break;
      case 0xFFFF:
          Ret=fInterruptRegister;
          break;
      default:
        std::cout << "[ERR] " << address << " unsupported !!\n";
        break;
    }
    #if defined MEMORY_RW_DEBUG
    if(address<0xFF00)
      std::cout << "Memory read @ 0x" << std::hex << address << " = 0x" << (short)Ret << std::dec << '\n';
    #endif
    return Ret;
  }

  unsigned short XMemory::ReadW(unsigned short address)
  {
    unsigned short Ret=0;
    Ret=(Read(address)|Read(address+1)<<8);
    return Ret;
  }

  void XMemory::Write(XCPU* cpu, unsigned short address, unsigned char val)
  {
    #if defined MEMORY_RW_DEBUG
    if(address<0xFF00)
      std::cout << "Memory write @ 0x" << std::hex << address << " = 0x" << (short)val << std::dec << '\n';
    #endif
    switch(address)
    {
      case 0x0000 ... 0x7FFF:    //Rom
        fRom.Write(address, val);
        break;
      case 0x8000 ... 0x9FFF:    //Video ram
        fVideoRam.Write(address, val);
        break;
      case 0xA000 ... 0xBFFF:    //Switchable ram
        fSwitchableRam.Write(address, val);
        break;
      case 0xC000 ... 0xFDFF:    //Internal ram
        fRam.Write(address, val);
        break;
      case 0xFE00 ... 0xFE9F:    //OAM
        fOAMMem[address&0xFF]=val;
        break;
      case 0xFEA0 ... 0xFEFF:    //Empty but unusable for I/O
        break;
      case 0xFF00 ... 0xFF4B:    //I/O Ports
      {
        int Port=address&0xFF;
        switch(Port)
        {
          case P1_REG: //Joypad
            //Read only
            break;
          case SB_REG:
          case TIMA_COUNT_REG:
          case TIMA_MOD_REG:
          case IF_REG:
          case SCY_REG:
          case SCX_REG:
          case WY_REG:
          case WX_REG:
            fIOPorts[Port]=val;
            break;
          case SC_REG:
            //Serial, not supported
            break;
          case DIV_REG:
            fIOPorts[Port]=0;
            //gbDivTicks = GBDIV_CLOCK_TICKS;
            break;
          case TAC_REG:
          {
            fIOPorts[Port]=val;
            gTimer=val&0x4;
            int Mode=val&0x3;
            switch(Mode)
            {
              case 0: gTicksCount=256; break;
              case 1: gTicksCount=4; break;
              case 2: gTicksCount=16; break;
              case 3: gTicksCount=64; break;
            }
          }
          break;
          case 0x10 ... 0x3F:    //Sound write
            //Not supported yet
            break;
          case LCDC_CONT_REG:
          {
            bool Value=val&0x80;
            if((fIOPorts[LCDC_CONT_REG]&0x80)&&(!Value)) //LCDC Off
            {
              fVideoRam.fCycles=0;
              fVideoRam.fMode=XVideoRam::eHBlank;
              fIOPorts[LCDC_STAT_REG]&=0xFC;
              fIOPorts[LY_REG]=0;
              #if defined DEBUG && defined VERBOSE
              std::cout << "LY_REG=0\n";
              #endif
              fVideoRam.CLS();
            }
            if((!(fIOPorts[LCDC_CONT_REG]&0x80))&&Value) //LCDC On
              fVideoRam.On(cpu, this);
            fIOPorts[LCDC_CONT_REG]=val;
          }
          break;
          case LCDC_STAT_REG:
            fIOPorts[Port]=(fIOPorts[Port]&0x07)|(val&0xF8);
            break;
          case LY_REG:
            #if defined DEBUG && defined VERBOSE
            std::cout << "WRITE FF44\n";
            #endif
            fIOPorts[LY_REG]=0;
            if(fIOPorts[LCDC_CONT_REG]&0x80) fVideoRam.On(cpu, this);
            break;
          case LYC_REG:
            fIOPorts[Port]=val;
            if(fIOPorts[LCDC_CONT_REG]&0x80)
            {
              if(fIOPorts[LY_REG]==fIOPorts[Port])
              {
                fIOPorts[LCDC_STAT_REG]|=4;
                if(fIOPorts[LCDC_STAT_REG]&0x40) fIOPorts[IF_REG]|=2;
              }
              else fIOPorts[IF_REG]&=0xFB;
            }
            break;
          case DMA_REG:
          {
            int Count=0xA0;      //40x32 bits
            unsigned short Dest=0xFE00;
            unsigned short Source=val*0x100;
            while(Count--) Write(cpu, Dest++,Read(Source++));
            fIOPorts[Port]=val;
          }
          break;
          case BGP_REG:
            fVideoRam.fBkgPalData[0]=val&0x03;
            fVideoRam.fBkgPalData[1]=(val&0x0C)>>2;
            fVideoRam.fBkgPalData[2]=(val&0x30)>>4;
            fVideoRam.fBkgPalData[3]=(val&0xC0)>>6;
            fIOPorts[Port]=val;
            break;
          case OBP0_REG:
            fVideoRam.fObjPal0Data[0]=val&03;
            fVideoRam.fObjPal0Data[1]=(val&0x0C)>>2;
            fVideoRam.fObjPal0Data[2]=(val&0x30)>>4;
            fVideoRam.fObjPal0Data[3]=(val&0xC0)>>6;
            fIOPorts[Port]=val;
            break;
          case OBP1_REG:
            fVideoRam.fObjPal1Data[0]=val&03;
            fVideoRam.fObjPal1Data[1]=(val&0x0C)>>2;
            fVideoRam.fObjPal1Data[2]=(val&0x30)>>4;
            fVideoRam.fObjPal1Data[3]=(val&0xC0)>>6;
            fIOPorts[Port]=val;
            break;
        }
      }
      break;
      case 0xFF4C ... 0xFF7F:    //Empty but unusable for I/O
        break;
      case 0xFF80 ... 0xFFFE:    //Internal Ram
        #if defined MEMORY_RW_DEBUG
          std::cout << "Memory write @ 0x" << std::hex << address << " = 0x" << (short)val << std::dec << '\n';
        #endif
        fInternalRam[address-0xFF80]=val;
        break;
      case 0xFFFF:
        fInterruptRegister=val&0x1F;
        break;
    }
  }
}
