/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "def.h"
#include "gameboy.h"
#include <fstream>
#include <iostream>
#include <memory.h>
#include <cstring>

#if defined PLATFORM_OS_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

//Globals
bool gTimer=false;
int gTicksCount=0;

namespace Hardware
{
  XGameboy::XGameboy()
  {
    Reset();
  }

  void XGameboy::Reset()
  {
    //Support eGB and eGBC
    fType=eGB;
    fScreenHeight=160;
    fScreenWidth=144;
    fWithSound=true;
    fWithFPS=true;
    fIsRunning=false;
    fPause=false;

    //Extern glob
    gTimer=false;
    gTicksCount=0;

    fCart.Reset();
    fMem.Reset(XCartridge::eROM_ONLY);
    //fCPU.Reset();
  }

  XGameboy::~XGameboy()
  {
  }

  void XGameboy::Quit()
  {
    if( (1<<fCart.fType) & (1<<XCartridge::eROM_MBC1_RAM_BATT | 1<<XCartridge::eROM_MBC2_BATTERY
      | 1<<XCartridge::eROM_MBC3_TIMER_BATT | 1<<XCartridge::eROM_MBC3_TIMER_RAM_BATT
      | 1<<XCartridge::eROM_MBC3_RAM_BATT | 1<<XCartridge::eROM_MBC5_RAM_BATT
      | 1<<XCartridge::eROM_MBC5_RUMBLE_SRAM_BATT | 1<<XCartridge::eROM_MMM01_SRAM_BATT
      | 1<<XCartridge::eROM_RAM_BATTERY))
    {
      #if defined LOG
      std::cout << "LOG - XGameboy: Support Save/Load game\n";
      #endif
      //SaveGame();
    }
  }

  void XGameboy::Run(const std::string& file)
  {
    Reset();
    LoadCartridge(file);
    fMem.fVideoRam.Init();
    //LCD On :)
    fMem.fVideoRam.On(&fCPU,&fMem);
    //Init();
    fIsRunning=true;

    #if defined PLATFORM_OS_LINUX
    #if defined WITH_X11
    XEvent Event;
    #endif
    #if defined WITH_SDL
    SDL_Event event;
    #endif
    while(fIsRunning && !fPause)
    {
      fCPU.Run(&fMem);
      if(fMem.fVideoRam.fRefresh)
      {
        fMem.fVideoRam.Blit();
        fMem.fVideoRam.fRefresh=false;
      }
      
      #if defined WITH_X11
      //XNextEvent(fMem.fVideoRam.fDisplay, &Event);
      switch (Event.type)
      {
        case KeyPress:
        {
          unsigned int KeyCode=((XKeyPressedEvent*)&Event)->keycode;
          char* Char=XKeysymToString(XKeycodeToKeysym(fMem.fVideoRam.fDisplay,KeyCode,0));
          #if defined LOG
          std::cout << "LOG - XGameboy: Char=" << Char << '\n';
          #endif
          fIsRunning=false;
        }
          break;

        case Expose:
          fMem.fVideoRam.Blit();
          break;
      } 
      #endif 
      #if defined WITH_SDL
      while(SDL_PollEvent(&event)) 
      {      
        switch (event.type) 
        {
          case SDL_QUIT:
            fIsRunning=false;
            break;
            //SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT,SDLK_x,SDLK_w,SDLK_RETURN,SDLK_RSHIFT
          case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
            {
              //if (key[kmap[PAD_START]]) gb_pad|=0x08; /* Start */
      //if (key[kmap[PAD_SELECT]]) gb_pad|=0x04; /* Select */
      //if (key[kmap[PAD_A]]) gb_pad|=0x01; /* A */
      //if (key[kmap[PAD_B]]) gb_pad|=0x02; /* B */
      //  } else {
      //      if ((joy_but[jmap[PAD_START]]) || (key[kmap[PAD_START]])) gb_pad|=0x08; /* Start */
      //      if ((joy_but[jmap[PAD_SELECT]]) || (key[kmap[PAD_SELECT]])) gb_pad|=0x04; /* Select */
      //      if ((joy_but[jmap[PAD_A]]) || (key[kmap[PAD_A]])) gb_pad|=0x01; /* A */
      //      if ((joy_but[jmap[PAD_B]]) || (key[kmap[PAD_B]])) gb_pad|=0x02; /* B */
      //    }
              case SDLK_UP:
                fMem.fIOPorts[P1_REG]|=0x40;
                break;
              case SDLK_DOWN:
                fMem.fIOPorts[P1_REG]|=0x80;
                break;
              case SDLK_LEFT:
                fMem.fIOPorts[P1_REG]|=0x20;
                break;
              case SDLK_RIGHT:
                fMem.fIOPorts[P1_REG]|=0x10;
                break;
            }
            break;
        }
      }
      #endif
    }
    #endif
    Quit();
  }

  void XGameboy::Pause()
  {
    fPause=!fPause;
  }

  void XGameboy::LoadCartridge(const std::string& file)
  {
    //Reset hardware components
    fCart.Reset();
    fMem.Reset(XCartridge::eROM_ONLY);

    //Open cartridge in binary mode
    #if defined LOG
    std::cout << "LOG - XGameboy: Loading : " << file << '\n';
    #endif
    std::fstream Cartridge(file.c_str(), std::ios_base::in |std::ios_base::binary);
    fCart.Load(&Cartridge);

    //Load ROM
    #if defined LOG
    std::cout << "LOG - XGameboy: New Rom size : " << (short)fCart.fRomSize << '\n';
    #endif
    fMem.fRom.SetSize(fCart.fRomSize);
    #if defined LOG
    std::cout << "LOG - XGameboy: Rom bank 0 transfer to internal Rom : 0x0\n";
    #endif
    memcpy(fMem.fRom.fMem,fCart.fMemory,ROM_BANK_SIZE);
    #if defined LOG
    std::cout << "LOG - XGameboy: ROM bank 1 transfer to internal Rom : 0x" << ROM_BANK_SIZE << "\n";
    #endif
    memcpy(fMem.fRom.fMem+ROM_BANK_SIZE,fCart.fMemory+ROM_BANK_SIZE,ROM_BANK_SIZE);
    int RomBanksCount=fMem.fRom.fBanksCount;
    #if defined LOG
    if(RomBanksCount>2) std::cout << "LOG - XGameboy: ROM bank 2 to " << std::dec << RomBanksCount << " transfer\n" << std::hex;
    #endif
    if(fCart.fType>=XCartridge::eROM_MBC1 && fCart.fType<=XCartridge::eROM_MBC5_RUMBLE_SRAM_BATT)
    {
      for(int i=2; i<RomBanksCount; ++i)
      {
        for(int j=0; j<ROM_BANK_SIZE; ++j)
        {
          #if defined LOG
          if(!j) std::cout << " 0x" << ROM_BANK_SIZE*i+j;
          #endif
          Cartridge.read((char*)(fMem.fRom.fMem+ROM_BANK_SIZE*i+j), 1);
        }
      }
    }

    #if defined LOG
    if(RomBanksCount>2) std::cout << '\n';
    #endif

    //Load RAM if available
    if( (1<<fCart.fType) & (1<<XCartridge::eROM_MBC1_RAM | 1<<XCartridge::eROM_MBC1_RAM_BATT
      | 1<<XCartridge::eROM_MBC3_TIMER_RAM_BATT | 1<<XCartridge::eROM_MBC3_RAM
      | 1<<XCartridge::eROM_MBC3_RAM_BATT | 1<<XCartridge::eROM_MBC5_RAM
      | 1<<XCartridge::eROM_MBC5_RAM_BATT | 1<<XCartridge::eROM_MBC5_RUMBLE_SRAM
      | 1<<XCartridge::eROM_MBC5_RUMBLE_SRAM_BATT | 1<<XCartridge::eROM_MMM01_SRAM
      | 1<<XCartridge::eROM_MMM01_SRAM_BATT | 1<<XCartridge::eROM_RAM | 1<<XCartridge::eROM_RAM_BATTERY))
    {
      #if defined LOG
      std::cout << "LOG - XGameboy: Support RAM : " << (short)fCart.fRamSize << '\n';
      #endif
      fMem.fRam.SetSize(fCart.fRamSize);
    }
    else
    {
      fMem.fRam.SetSize(2);
    }

    //If support save (battery inside cartridge)
    if( (1<<fCart.fType) & (1<<XCartridge::eROM_MBC1_RAM_BATT | 1<<XCartridge::eROM_MBC2_BATTERY
      | 1<<XCartridge::eROM_MBC3_TIMER_BATT | 1<<XCartridge::eROM_MBC3_TIMER_RAM_BATT
      | 1<<XCartridge::eROM_MBC3_RAM_BATT | 1<<XCartridge::eROM_MBC5_RAM_BATT
      | 1<<XCartridge::eROM_MBC5_RUMBLE_SRAM_BATT | 1<<XCartridge::eROM_MMM01_SRAM_BATT
      | 1<<XCartridge::eROM_RAM_BATTERY))

    {
      #if defined LOG
      std::cout << "LOG - XGameboy: Support Save/Load game\n";
      #endif
      //LoadGame();
    }

    //Video Ram
    if(fType==eGBC)
    {
      fMem.fVideoRam.fColor=true;
      fMem.fVideoRam.fBanksCount=2;
    }
    else
    {
      fMem.fVideoRam.fColor=false;
      fMem.fVideoRam.fBanksCount=1;
    }
    fMem.fVideoRam.SetSize();

    //Switchable Ram
    if(fType==eGBC) fMem.fSwitchableRam.fBanksCount=8;
    else fMem.fSwitchableRam.fBanksCount=2;
    fMem.fSwitchableRam.SetSize();
    //Then close cartridge.
    Cartridge.close();
  }
}
