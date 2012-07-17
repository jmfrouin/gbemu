/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "videoram.h"
#include <cstdlib>
#include <memory.h>
#include "gameboy.h"
#include "memory.h"

unsigned char gMask[8];
unsigned char gShift[8];

namespace Hardware
{
  extern XGameboy* GBInst;

  XVideoRam::XVideoRam():
  #if defined PLATFORM_OS_LINUX
  #if defined WITH_X11
  fDisplay(0), fScreen(0), fDepth(0), fGC(0), fWin(0), fPixmap(0),
  #endif

  #if defined WITH_SDL
  fScreen(0),
  #endif
  #endif
  fBuffer(0), fColor(false), fRefresh(false), fBackBuffer(0)
  {
  }

  XVideoRam::~XVideoRam()
  {
    if(fMem) delete[] fMem;
    //if(fBackBuffer) delete[] fBackBuffer;
    Destroy();
  }

  void XVideoRam::Destroy()
  {
    #if defined PLATFORM_OS_LINUX
    #if defined WITH_X11
    if(fGC) XFreeGC(fDisplay, fGC);
    if(fWin) XDestroyWindow(fDisplay, fWin);
    if(fDisplay) XCloseDisplay(fDisplay);
    //Delete fBuffer too !!
    if(fPixmap) XDestroyImage(fPixmap);
    fGC=0;
    fWin=0;
    fDisplay=0;
    fScreen=0;
    fDepth=0;
    fPixmap=0;
    #endif
    #endif
  }

  void XVideoRam::Reset()
  {
    fBanksCount=0;
    fCurrentBank=0;

    #if defined PLATFORM_OS_LINUX
    #if defined WITH_X11
    Destroy();
    fGC=0;
    fFS=0;
    #endif

    #if defined WITH_SDL
    #endif
    #endif

    fWidth=fHeight=0;
    for(int i=4;--i>=0;) fObjPal0Data[i]=fObjPal1Data[i]=i;

    fBkgPalData[0]=0;
    fBkgPalData[1]=fBkgPalData[2]=fBkgPalData[3]=3;

    //Classic gameboy color palette
    fGBPalCol[0]=0xBD31;
    fGBPalCol[1]=0x93EB;
    fGBPalCol[2]=0x6286;
    fGBPalCol[3]=0x20C2;

    /* LCD Control
    Mode 0 is present between 201-207 clks,	2 about 77-83 clks, and 3 about 169-175 clks.
    A complete cycle through these states takes 456 clks. VBlank lasts 4560 clks.
    */
    fM1Cycles=456;
    fM2Cycles=80;
    fM3Cycles=170;

    // A complete screen refresh occurs every 70224 clks
    fVBlankCycles=0x11250;

    fCycles=fM2Cycles;
    fTiming=0;
    fFactor=1;
    for(int i=160; --i>=0;) fPalLine[i]=fBkgPalData;
  }

  void XVideoRam::On(XCPU* cpu, XMemory* mem)
  {
    fMode=eEndVBlank;
    fWinCurrentLine=0;
    mem->fIOPorts[LY_REG]=0;
    mem->fIOPorts[LCDC_CONT_REG]|=0x80;
    fLineInc=0;
    fInitPal=true;
    CLS();
    fCycles=Update(cpu,mem);
  }

  void XVideoRam::CLS()
  {
    for(int i=GB_SCREEN_WIDTH*GB_SCREEN_HEIGHT; --i>=0;)
    {  
      #if defined PLATFORM_OS_LINUX
      #if defined WITH_X11
      fBuffer[i]=(fColor?0:fPalBkg[0]);
      #endif
      #if defined WITH_SDL
      SDL_FillRect(fBuffer,0,0);
      #endif
      #endif
      fBackBuffer[i]=0;
    }
    fRefresh=true;
  }

  unsigned short XVideoRam::Update(XCPU* cpu,XMemory* mem)
  {
    unsigned short Ret=0;
    unsigned char SkipFrame;

    #if defined DEBUG && defined VERBOSE
    std::cout <<"mode=" << fMode << '\n';
    #endif

    if(fLineInc)
    {
      mem->fIOPorts[LY_REG]++;
      #if defined DEBUG && defined VERBOSE
      std::cout << "LY_REG=" << (short)mem->fIOPorts[LY_REG] << '\n';
      #endif
      mem->fIOPorts[LCDC_STAT_REG]&=0xF8;
      if(mem->fIOPorts[LY_REG]       && mem->fIOPorts[LY_REG]==mem->fIOPorts[LYC_REG])     mem->fIOPorts[LCDC_STAT_REG]|=0x04;
      if(mem->fIOPorts[LY_REG]==0x99 && mem->fIOPorts[LYC_REG]==0)                mem->fIOPorts[LCDC_STAT_REG]|=0x04;
    }

    if(mem->fIOPorts[LY_REG]==0x90 && fMode==eOAM)
    {  
      fMode=eVBlank;
      mem->fIOPorts[IF_REG]|=VBLANK_INT;
    }

    if(mem->fIOPorts[LY_REG]==0 && fMode==eEndVBlank)
    {
      fRefresh=true;
      mem->fIOPorts[LCDC_STAT_REG]&=0xF8;
      fMode=eOAM;
      fWinCurrentLine=0;
    }
    if(mem->fIOPorts[LY_REG]==0x99 && fMode==eLine99)
    {
      #if defined DEBUG && defined VERBOSE
      std::cout << "LY_REG==0\n";
      #endif
      mem->fIOPorts[LY_REG]=0;
      fMode=eEndVBlank;
    }
    else
    {
      if(mem->fIOPorts[LY_REG]==0x99) fMode=eLine99;
    }

    switch(fMode)
    {
      case eHBlank:
        if(mem->fIOPorts[LCDC_STAT_REG]&0x08) mem->fIOPorts[IF_REG]|=LCDC_INT;
        Draw(mem);
        fRefresh=true;
        Ret=fColor?gCycle_Tab[1][0]:gCycle_Tab[0][0];
        fMode=eOAM;
        fLineInc=1;
        mem->fIOPorts[LCDC_STAT_REG]&=0xFC;
        break;
      case eVBlank:
        if(mem->fIOPorts[LCDC_STAT_REG]&0x40 && mem->fIOPorts[LCDC_STAT_REG]&0x04) mem->fIOPorts[IF_REG]|=LCDC_INT;
        mem->fIOPorts[LCDC_STAT_REG]|=0x01;
        Ret=fColor?gCycle_Tab[1][1]:gCycle_Tab[0][1];
        fLineInc=1;
        break;
      case eOAM:
        mem->fIOPorts[LCDC_STAT_REG]|=0x02;
        fLineInc=0;
        if(mem->fIOPorts[LCDC_STAT_REG]&0x40 && mem->fIOPorts[LCDC_STAT_REG]&0x04) mem->fIOPorts[IF_REG]|=LCDC_INT;
        fMode=eVRAM;
        Ret=fColor?gCycle_Tab[1][2]:gCycle_Tab[0][2];
        SpriteCount(mem);
        break;
      case eVRAM:
        mem->fIOPorts[LCDC_STAT_REG]|=0x03;
        Ret=fColor?gCycle_Tab[1][3]:gCycle_Tab[0][3];
        fMode=eHBlank;
        fM3Cycles=Ret;
        fFactor=(GB_SCREEN_WIDTH*1.0)/(double)fM3Cycles;
        if(fInitPal)
        {
          for(int i=160; --i>=0;) fPalLine[i]=fBkgPalData;
          fBkgPalData[0]=mem->fIOPorts[BGP_REG]&3;
          fBkgPalData[1]=(mem->fIOPorts[BGP_REG]>>2)&3;
          fBkgPalData[2]=(mem->fIOPorts[BGP_REG]>>4)&3;
          fBkgPalData[3]=(mem->fIOPorts[BGP_REG]>>6)&3;
          fInitPal=false;
        }
        break;
      case eEndVBlank:
        mem->fIOPorts[LCDC_STAT_REG]|=0x01;
        if(mem->fIOPorts[LCDC_STAT_REG]&0x40 && mem->fIOPorts[LCDC_STAT_REG]&0x04) mem->fIOPorts[IF_REG]|=LCDC_INT;
        Ret=(fColor?gCycle_Tab[1][1]:gCycle_Tab[0][1])-(fColor?gCycle_Tab[1][4]:gCycle_Tab[0][4]);
        fLineInc=0;
        break;
      case eBegLine90:
        fLineInc=0;
        mem->fIOPorts[LCDC_STAT_REG]|=0x01;
        if(mem->fIOPorts[LCDC_STAT_REG]&0x40 && mem->fIOPorts[LCDC_STAT_REG]&0x04) mem->fIOPorts[IF_REG]|=LCDC_INT;
        Ret=0x18;
        fMode=eEndLine90;
        break;
      case eEndLine90:
        fLineInc=1;
        if(mem->fIOPorts[LCDC_STAT_REG]&0x10) mem->fIOPorts[IF_REG]|=LCDC_INT;
        Ret=(fColor?gCycle_Tab[1][1]:gCycle_Tab[0][1])-0x18-0x08;
        mem->fIOPorts[LCDC_STAT_REG]|=0x01;
        fMode=eVBlank;
        mem->fIOPorts[IF_REG]|=VBLANK_INT;
        break;
      case eLine99:
        mem->fIOPorts[LCDC_STAT_REG]|=0x01;
        Ret=fColor?gCycle_Tab[1][4]:gCycle_Tab[0][4];
        fLineInc=0;
        break;
    }
    return Ret;
  }

  void XVideoRam::SpriteCount(XMemory* mem)
  {
    #if defined DEBUG && defined VERBOSE
    std::cout << std::hex << (short)mem->fIOPorts[LCDC_CONT_REG] << '\n' << std::dec;
    #endif
    if(!(mem->fIOPorts[LCDC_CONT_REG]&0x02)) { fSpriteCount=0; return; }

    unsigned char SizeY;

    SizeY=(mem->fIOPorts[LCDC_CONT_REG]&0x04)?0x10:0x08;

    fSpriteCount=0;
    for(int i=40;--i>=0;)
    {
      short X,Y,NoTile,Att;
      unsigned char YOff,XOff;
      Y=mem->fOAMMem[fSpriteCount+0];
      X=mem->fOAMMem[fSpriteCount+1];
      NoTile=mem->fOAMMem[fSpriteCount+2];
      Att=mem->fOAMMem[fSpriteCount+3];

      Y-=0x10;
      YOff=mem->fIOPorts[LY_REG]-Y;

      if((mem->fIOPorts[LY_REG]>=Y)&&(YOff<SizeY)&&(X-8<GB_SCREEN_WIDTH))
      {
        if(X<8) XOff=8-X;
        else XOff=0;
        fSprites[fSpriteCount].fSizeY=SizeY;
        fSprites[fSpriteCount].fX=X-8;
        fSprites[fSpriteCount].fY=Y;
        fSprites[fSpriteCount].fXOff=XOff;
        fSprites[fSpriteCount].fYOff=YOff;
        fSprites[fSpriteCount].fXFlip=(Att&0x20)>>5;
        fSprites[fSpriteCount].fYFlip=(Att&0x40)>>6;
        fSprites[fSpriteCount].fPalCol=Att&0x70;
        if(Att&0x10) fSprites[fSpriteCount].fPal=1;
        else fSprites[fSpriteCount].fPal=0;
        fSprites[fSpriteCount].fPage=(Att&0x08)>>3;
        fSprites[fSpriteCount].fPriority=(Att&0x80);
        if(SizeY==0x10) fSprites[fSpriteCount].fNoTile=NoTile&0xFE;
        else fSprites[fSpriteCount].fNoTile=NoTile;
        fSpriteCount++;
        if(fSpriteCount==10) break; //Hardware limitation
      }
    }
    #if defined DEBUG && defined VERBOSE
    std::cout << (short)fSpriteCount << " sprites\n";
    #endif
  }

  void XVideoRam::SetSize()
  {
    if(fMem)
    {
      #if defined LOG
      std::cout << "LOG - XVideoRAM: Deletting memory @ " << std::hex << (short*)fMem << '\n';
      #endif
      delete[] fMem;
    }

    fMem=new unsigned char[fBanksCount*RAM_BANK_SIZE];

    #if defined LOG
    std::cout << std::hex << "LOG - XVideoRAM: Banks count : " << fBanksCount << '\n';
    std::cout << "LOG - XVideoRAM: Allocate " << fBanksCount*RAM_BANK_SIZE << " @ " << (short*)fMem << '\n';
    #endif
    fCurrentBank=0;
  }

  EErrors XVideoRam::Init()
  {
    #if defined PLATFORM_OS_LINUX
    #if defined WITH_X11
    //Open X11surface
    fDisplay = XOpenDisplay(getenv("DISPLAY"));
    if(!fDisplay)
      return eSurfaceCannotOpenDisplay;
    
    //Get system infos
    fScreen = DefaultScreen(fDisplay);
    fDepth = DefaultDepth(fDisplay, fScreen);
    
    // Window attributes
    fWA.border_pixel = BlackPixel(fDisplay, fScreen);
    fWA.background_pixel = WhitePixel(fDisplay, fScreen);
    fWA.override_redirect = False;

    // Background and foreground colors
    fBackground = WhitePixel(fDisplay,fScreen);
    fForeground = BlackPixel(fDisplay, fScreen);

    fHint.x = 100;
    fHint.y = 100;
    fHint.width = GB_SCREEN_WIDTH;
    fHint.height = GB_SCREEN_HEIGHT;
    fHint.flags=PPosition;

    // We create the main window
    fWin = XCreateWindow(fDisplay, DefaultRootWindow(fDisplay), 100, 100, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 0, fDepth, InputOutput, CopyFromParent,CWBackPixel | CWBorderPixel | CWOverrideRedirect, &fWA);

    if(fWin==None)
    {
      std::cout << "Error XSurface : Can't create window\n";
      XCloseDisplay(fDisplay);
      return eSurfaceCannotCreateWindow;
    }

    // Recommandation for wm
    XSetNormalHints(fDisplay, fWin, &fHint);

    // Graphic context
    fGC = XCreateGC(fDisplay, fWin, 0, NULL);

    // We set some properties
    XSetBackground(fDisplay, fGC, fBackground);
    XSetForeground(fDisplay, fGC, fForeground);
    XSetLineAttributes(fDisplay, fGC, 2, 0, 0, 0);

    // We display the main window
    XMapRaised(fDisplay, fWin);

    // Evenment filter
    XSelectInput(fDisplay, fWin, ButtonPressMask|KeyPressMask|ExposureMask);

    // Font setting
    if((fFS = XLoadQueryFont(fDisplay, "fixed")) == NULL)
    {
      std::cout << "Error - XSurface: Not possible to query the font\n";
      return eSurfaceCannotGetFont;
    }

    XSetStandardProperties(fDisplay, fWin, NAME, NAME, None, (char**)0, 0, &fHint);

    // Evenments filter
    XSelectInput(fDisplay, fWin, ButtonPressMask|KeyPressMask|ExposureMask);

    // Display it
    XMapWindow(fDisplay, fWin);

    //Now create XImage video buffer :D
    //For X300 : Here under slickedit with projects panel visible & gkrellm :)
    XWindowAttributes XWA;
    XGetWindowAttributes(fDisplay, fWin, &XWA);

    fBuffer=(int*)malloc(GB_SCREEN_WIDTH*GB_SCREEN_HEIGHT*sizeof(int));
    memset(fBuffer, 1, GB_SCREEN_WIDTH*GB_SCREEN_HEIGHT*sizeof(int));

    /*
     * bitmap_pad
    Specifies the quantum of a scanline (8, 16, or 32). In other words, the start of one scanline is separated in client memory from the start of the next scanline by an integer multiple of this many bits.
    * bytes_per_line
    Specifies the number of bytes in the client image between the start of one scanline and the start of the next.
    */
    fPixmap=XCreateImage(fDisplay, XWA.visual, fDepth, ZPixmap, 0, (char*)fBuffer, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 32, 0);
    #endif
    #if defined WITH_SDL
    if(SDL_Init(SDL_INIT_VIDEO)<0) return eSDLInitVideoFailed;
    fScreen=SDL_SetVideoMode(GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, GB_SCREEN_DEPTH, /*SDL_FULLSCREEN|*/SDL_HWSURFACE);
    if(!fScreen)
    {  
      SDL_Quit();
      return eSDLCannotSetVideoMode;
    }
     
    fBuffer=SDL_CreateRGBSurface(SDL_SWSURFACE,GB_SCREEN_WIDTH,GB_SCREEN_HEIGHT+1,GB_SCREEN_DEPTH,0xf800,0x7e0,0x1f,0x00);
    if(!fBuffer)
    {      
      SDL_Quit();
      return eSDLCannotCreateRGBSurface;
    }

    SDL_WM_SetCaption(NAME,NULL); 
    //No need fBuffer under SDL
    #endif
    #endif
    fBackBuffer=new unsigned char[GB_SCREEN_WIDTH*GB_SCREEN_HEIGHT];

    //Mask&Shift
    for(int i=8; --i>=0;) gMask[i]=1<<(gShift[i]=7-i);

    return eNoError;
  }

  void XVideoRam::Blit()
  {
    //std::cout << "Blit\n";

    //Now something to see :D
    /*static int j=1;
    for(int i=GB_SCREEN_WIDTH*GB_SCREEN_HEIGHT; --i>=0;)
      *((unsigned int*)fBuffer->pixels+i)=i%j?GB_WHITE_PIXEL:GB_BLACK_PIXEL;
    j++;*/

    #if defined PLATFORM_OS_LINUX
    #if defined WITH_X11
    XPutImage(fDisplay, fWin, fGC, fPixmap, 0, 0, 0, 0, GB_SCREEN_WIDTH-1, GB_SCREEN_HEIGHT-1);
    #endif

    #if defined WITH_SDL
    if(SDL_MUSTLOCK(fScreen) && (SDL_LockSurface(fScreen)<0)) return;

    SDL_Rect Rect;
    Rect.x=0;
    Rect.y=0;
    Rect.h=GB_SCREEN_HEIGHT;
    Rect.w=GB_SCREEN_WIDTH;
    SDL_BlitSurface(fBuffer,&Rect,fScreen,&Rect);
    //SDL_Flip(fScreen);

    //Now something to see :D
    /*static int j=1;
    for(int i=GB_SCREEN_WIDTH*GB_SCREEN_HEIGHT; --i>=0;)
      *((unsigned int*)fScreen->pixels+i)=i%j?GB_WHITE_PIXEL:GB_BLACK_PIXEL;
    j++;*/

    if(SDL_MUSTLOCK(fScreen)) SDL_UnlockSurface(fScreen);
    SDL_Flip(fScreen);
    #endif
    #endif
  }

  unsigned char XVideoRam::Read(unsigned short address)
  {
    #if defined DEBUG && defined VERBOSE
    std::cout << "Read @ 0x" << std::hex << address << " = " << (short)fMem[address-VIDEO_RAM_START] << '\n';
    #endif
    return fMem[address-VIDEO_RAM_START];
  }

  void XVideoRam::Write(unsigned short address, unsigned char val)
  {
    #if defined DEBUG && defined VERBOSE
    std::cout << "Write @ 0x" << std::hex << address << " = " << (short)val << '\n';
    #endif
    fMem[address-VIDEO_RAM_START]=val;
  }

  void XVideoRam::Draw(XMemory* mem)
  {
    fRefresh=true;
    #if defined WITH_SDL
    unsigned short* Buffer=(unsigned short*)fBuffer->pixels+mem->fIOPorts[LY_REG]*(fBuffer->pitch>>1);
    if(SDL_MUSTLOCK(fBuffer) && SDL_LockSurface(fBuffer)<0) std::cout << "[ERR]Cannot lock SDL surface\n";
 
    if(mem->fIOPorts[LCDC_CONT_REG]&0x01) DrawBack(mem,Buffer);
    if(mem->fIOPorts[LCDC_CONT_REG]&0x20) DrawWin(mem,Buffer);
    if(mem->fIOPorts[LCDC_CONT_REG]&0x02) DrawObj(mem,Buffer);
  
    if(SDL_MUSTLOCK(fBuffer)) SDL_UnlockSurface(fBuffer);
    #endif
  }

  void XVideoRam::DrawBack(XMemory* mem,unsigned short* buffer)
  {
    unsigned char* TB;
    unsigned char* TP;
    short Y,X,SX,SY;
    short NoTile;
    unsigned char C;
     
    if(mem->fIOPorts[LCDC_CONT_REG]&0x08) TB=&fMem[0x1C00];
    else TB=&fMem[0x1800];

    Y=mem->fIOPorts[LY_REG];
    SY=mem->fIOPorts[SCY_REG]+Y;

    TB+=((SY>>3)<<5)&0x3FF;
    int I=mem->fIOPorts[SCX_REG]>>3;

    NoTile=TB[I&0x1F];
    if (!(mem->fIOPorts[LCDC_CONT_REG]&0x10)) NoTile=256+(signed char)NoTile;
    TP=&fMem[NoTile<<4];
    TP+=(SY&0x07)<<1;
          
    X=0;
    for(SX=mem->fIOPorts[SCX_REG]&0x07;SX<8;SX++,X++) 
    {
      C=GetPixel(TP,SX);
      buffer[X]=fGBPalCol[fPalLine[X][C]];
      fBackBuffer[X+GB_SCREEN_WIDTH*mem->fIOPorts[LY_REG]]=C;
    }

    I++;
    for(;X<160;X+=8,I++) 
    {
      NoTile=TB[I&0x1F];
      if (!(mem->fIOPorts[LCDC_CONT_REG]&0x10)) NoTile=256+(signed char)NoTile;
      TP=&fMem[NoTile<<4];
      TP+=(SY&0x07)<<1;
      for(SX=0;SX<8 && X+SX<160;SX++) 
      {
        C=GetPixel(TP,SX);
        buffer[X+SX]=fGBPalCol[fPalLine[X+SX][C]];
        fBackBuffer[X+SX+GB_SCREEN_WIDTH*mem->fIOPorts[LY_REG]]=C;
      }
    }
  }

  void XVideoRam::DrawWin(XMemory* mem,unsigned short* buffer)
  {
    std::cout << "Draw win\n";
    unsigned char* TB;
    short Y,X,SX;
    short NoTile;
    unsigned char C;
     
    if(mem->fIOPorts[LCDC_CONT_REG]&0x40) TB=&fMem[0x1C00];
    else TB=&fMem[0x1800];
    
    if(mem->fIOPorts[WX_REG]>=166) return;
    
    if(mem->fIOPorts[LY_REG]>=mem->fIOPorts[WX_REG]) 
    { 
      TB+=((fWinCurrentLine>>3)<<5);
      Y=(fWinCurrentLine&0x07)<<1;
      X=(((mem->fIOPorts[WX_REG]-7)<0)?0:(mem->fIOPorts[WX_REG]-7));
      for(int i=0;X<160;X+=8,i++) 
      {
        NoTile=TB[i];
        if (!(mem->fIOPorts[LCDC_CONT_REG]&0x10)) NoTile=256+(signed char)NoTile;
        for(SX=0;SX<8 && (X+SX)<160;SX++) 
        {
          buffer[X+SX]=0x1234;
          //back_col[X+SX][CURLINE]=c;
        }
      }
      fWinCurrentLine++;
    }  
  }

  void XVideoRam::DrawObj(XMemory* mem,unsigned short* buffer)
  {
    for(int i=fSpriteCount;--i>=0;) 
    DrawSprite(mem,buffer,i);
  }

  void XVideoRam::DrawSprite(XMemory* mem,unsigned short* buffer, int spritenb)
  {
    std::cout << "Draw sprites\n";
    unsigned char* TP;
    Uint8 c;
    
    XGBSprite* Sprite=&fSprites[spritenb];
    TP=&fMem[Sprite->fNoTile<<4];
    std::cout << std::hex << "TP=" << (short*)TP << '\n';
    if (!Sprite->fYFlip) TP+=(Sprite->fYOff<<1);
    else TP+=(Sprite->fSizeY-1-Sprite->fYOff)<<1;
  
    for(int i=Sprite->fXOff;i<8;i++) 
    {
      unsigned char Bit;
      if (!Sprite->fXFlip) Bit=i;
      else Bit=7-i;	
      unsigned char C=GetPixel(TP,Bit);
      if (C) 
      {
        if (!(Sprite->fPriority)) buffer[Sprite->fX+i]=fBkgPalData[fPalLine[Sprite->fPal][C]];
        else 
        {
          if (!(fBackBuffer[Sprite->fX+i+GB_SCREEN_WIDTH*mem->fIOPorts[LY_REG]]))
            buffer[Sprite->fX+i]=fBkgPalData[fPalLine[Sprite->fPal][C]];
        }
      }
    }
  }

  unsigned char XVideoRam::GetPixel(unsigned char* tp, int pos)
  {
    unsigned char Ret;
    Ret=((*tp)&gMask[pos])>>gShift[pos];
    Ret|=(((*(tp+1))&gMask[pos])>>gShift[pos])<<1;
    return Ret;
  }
}
