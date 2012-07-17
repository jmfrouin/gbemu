/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __VIDEORAM_H__
#define __VIDEORAM_H__

#include "mem.h"
#include "def.h"
#include <string>

#if defined PLATFORM_OS_LINUX
#if defined WITH_X11
//apt-get install libx11-dev
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#if defined WITH_SDL
//apt-get install libsdl1.2-dev
#include <SDL/SDL.h>
#endif
#endif

static const unsigned short gCycle_Tab[2][5]=
{
  {204,456,80,172,80},          /*GB*/
  {204*2,456*2,80*2,172*2,80*2} /*GBC*/
};

namespace Hardware
{
  class XMemory;
  class XCPU;
  /*!
   * @brief Video ram of GB
   * @note --------------------------- 0xA000
   * @note 8kB Video RAM
   * @note --------------------------- 0x8000
   */
  class XVideoRam : public IMem
  {
    public:
      class XGBSprite
      {
        public:
          short fX,fY;
          unsigned char fXOff,fYOff;
          unsigned char fSizeY;
          unsigned char fXFlip, fYFlip;
          unsigned char fPage, fPalCol;
          unsigned short fNoTile;
          unsigned char fPal;
          unsigned char fPriority;
      };

      enum EMode
      {
        eHBlank,
        eVBlank,
        eOAM,
        eVRAM,
        eEndVBlank,
        eBegLine90,
        eEndLine90,
        eLine99
      };

      XVideoRam();
      ~XVideoRam();
      EErrors Init();
      void Reset();
      void SetSize();
      void Blit();

      //From IMen
      unsigned char Read(unsigned short address);
      void Write(unsigned short address, unsigned char val);

      /*!
       * @brief Power on LCD :D
       * @author snoogie (2/15/2009)
       */
      void On(XCPU* cpu,XMemory* mem);

      /*!
       * @brief Update LCD infos.
       * @author snoogie (2/15/2009)
       * @return unsigned short Cycles.
       */
      unsigned short Update(XCPU* cpu,XMemory* mem);

      /*!
       * @brief Clear screen :D
       * @author snoogie (2/15/2009)
       */
      void CLS();

      /*!
       * @brief Draw screen :)
       * @author snoogie (3/2/2009)
       */
      void Draw(XMemory* mem);

    private:

      /*!
       * @brief Count sprites
       * @author snoogie (2/15/2009)
       * @param mem For OAM access.
       */
      void SpriteCount(XMemory* mem);

      void Destroy();

      /*!
       * @brief Draw background.
       * @author snoogie (3/2/2009)
       * @param buffer Draw in this buffer.
       */
      void DrawBack(XMemory* mem,unsigned short* buffer);

      /*!
       * @brief Draw tiles.
       * @author snoogie (3/2/2009)
       * @param buffer Draw in this buffer.
       */
      void DrawWin(XMemory* mem,unsigned short* buffer);

      /*!
       * @brief Draw objects.
       * @author snoogie (3/2/2009)
       * @param buffer Draw in this buffer.
       */
      void DrawObj(XMemory* mem,unsigned short* buffer);

      /*!
       * @brief Draw sprites.
       * @author snoogie (3/2/2009)
       * @param buffer Draw in this buffer. 
       * @param spritenb Sprite index. 
       */
      void DrawSprite(XMemory* mem,unsigned short* buffer,int spritenb);

      /*!
       * @brief Get Pixel.
       * @author snoogie (3/8/2009)
       * @param pos 
       * @return unsigned char 
       */
      unsigned char GetPixel(unsigned char* tp,int pos);

    private:
      friend class XGameboy;
      friend class XMemory;
      friend class XCPU;

      #if defined PLATFORM_OS_LINUX
      #if defined WITH_X11
      Display* fDisplay;
      int fScreen;
      int fDepth;
      GC fGC;
      XFontStruct* fFS;
      Window fWin;
      XSetWindowAttributes fWA; // Window Attributes
      XSizeHints fHint;
      std::string fFont; // Current font
      XImage* fPixmap;
      int* fBuffer;
      #endif
      #if defined WITH_SDL
      SDL_Surface* fScreen;
      SDL_Surface* fBuffer;
      #endif
      #endif

      // Background & foreground colors
      unsigned long fBackground;
      unsigned long fForeground;
      int fWidth;
      int fHeight;

      //LCD Control
      short fCycles;
      EMode fMode;
      unsigned short fM1Cycles;
      unsigned short fM2Cycles;
      unsigned short fM3Cycles;

      //Sprites: GB can manage 40 sprites, but due to hardware limitation only 10 are handle.
      XGBSprite fSprites[10];
      unsigned char fSpriteCount;

      unsigned char fLineInc;
      unsigned int  fVBlankCycles;
      unsigned char fTiming;
      unsigned char fWinCurrentLine;
      double fFactor;

      //Palettes
      bool fInitPal;
      unsigned char* fPalLine[160];

      //----------------------------
      //GB classic only
      //----------------------------
      unsigned char fBkgPalData[4];
      unsigned char fObjPal0Data[4];
      unsigned char fObjPal1Data[4];
      unsigned short fGBPalCol[4];
      //----------------------------

      //Internal use
      bool fColor;
      bool fRefresh;

      //Background buffer
      unsigned char* fBackBuffer;
  };
}
#endif                           //__VIDEORAM_H__
