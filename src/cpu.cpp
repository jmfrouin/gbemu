/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "cpu.h"
#include <iostream>
#include "gameboy.h"

namespace Hardware
{
  XCPU::XCPU()
  {
    Reset(0);
  }

  void XCPU::Reset(int gb)
  {
    switch(gb)
    {
      default:                   //A=$01-GB/SGB
      case XGameboy::eGB:
      case XGameboy::eSGB:
        fAF=0x0100;
        break;
      case XGameboy::eGBP:       //$FF-GBP
        fAF=0xFF00;
        break;
      case XGameboy::eGBC:       //$11-GBC
        fAF=0x1100;
        break;
    }
    fAF|=0xB0;                   //On power up
    fBC=0x0013;                  //On power up
    fDE=0x00D8;                  //On power up
    fHL=0x014D;                  //On power up
    fSP=0xFFFE;                  //On power up
    fPC=0x100;                   //On power up
    fSpeed=eSimple;
    fState=eHaltState;
    fIntFlag=0;
    fEIFlag=1;
    fCycles=0;
  }

  XCPU::~XCPU()
  {
  }

  void XCPU::Run(XMemory* mem)
  {
    unsigned char Cycles=0;
    fState=eRunState;

    bool Temp;

    if(mem->fIOPorts[IF_REG]&VBLANK_INT) Temp=Interrupt(mem, VBLANK_INT);
    if((mem->fIOPorts[IF_REG]&LCDC_INT)&&(!Temp)) Temp=Interrupt(mem, LCDC_INT);
    if((mem->fIOPorts[IF_REG]&TIMEOWFL)&&(!Temp)) Temp=Interrupt(mem, TIMEOWFL);

    if(fState!=eHaltState) Cycles=Exec(mem);
    else Cycles=4;

    if(mem->fIOPorts[LCDC_CONT_REG]&0x80)
    {
      mem->fVideoRam.fCycles-=Cycles;
      if(mem->fVideoRam.fCycles<=0)
        mem->fVideoRam.fCycles+=mem->fVideoRam.Update(this,mem);
    }

    fCycles+=Cycles;
  }

  void XCPU::Display(XMemory* mem, unsigned short opcode)
  {
    std::cout << std::hex << "0x" << opcode;
    std::cout << " [PC=0x" << fPC-1 << "] ";
    std::cout << std::hex;
    std::cout << "AF=0x" << fAF << ' ';
    std::cout << "BC=0x" << fBC << ' ';
    std::cout << "DE=0x" << fDE << ' ';
    std::cout << "HL=0x" << fHL << ' ';
    std::cout << "SP=0x" << fSP << ' ';
    std::cout << "0x" << (short)mem->fInterruptRegister << ' ';
    static int count=0;
    std::cout << std::dec << "Count = " << ++count << '\n';
  }

  //TODO: Check OP codes cycle
  void XCPU::Test(XMemory* mem)
  {
    /*fAF &= 0x00FF;
    fAF |= 0xDD00;
    fAF &= 0xFF00;
    fAF |= 0x00CC;*/
    //  std::cout << "LOG: Gameboy Classic:\n";
    //  Reset(XGameboy::eGB);
    //  Display();
    //  std::cout << "LOG: Super Gameboy:\n";
    //  Reset(XGameboy::eSGB);
    //  Display();
    //  std::cout << "LOG: Gameboy Pocket:\n";
    //  Reset(XGameboy::eGBP);
    //  Display();
    //  std::cout << "LOG: Gameboy Color:\n";
    //  Reset(XGameboy::eGBC);
    //  Display();
    std::cout << "Test Exec\n";

    std::cout << "Test SetFlag: ";
    fAF=0;
    SetFlag(ZFLAG);
    SetFlag(NFLAG);
    SetFlag(HFLAG);
    SetFlag(CFLAG);
    std::cout << (fAF==0x00F0?"OK\n":"!OK\n");

    std::cout << "Test UnSetFlag: ";
    UnSetFlag(CFLAG);
    UnSetFlag(HFLAG);
    UnSetFlag(NFLAG);
    UnSetFlag(ZFLAG);
    std::cout << (!fAF?"OK\n":"!OK\n");

    //8Bit loads: LD nn, n : Put value nn into n
    mem->Write(this, fPC, eLD_B_N);
    Exec(mem);

    //8Bit loads: LD r1,r2 : Put value r2 into r1
    mem->Write(this, fPC, eLD_A_B);
    Exec(mem);

    mem->Write(this, fPC, eLD_A_C);
    Exec(mem);

    fAF=0x1234;
    fHL=0x2;
    mem->Write(this, fPC, eLD_A_HL);
    Exec(mem);

    mem->Write(this, fPC, eLD_B_C);
    Exec(mem);

    fBC=9876;
    mem->Write(this, fPC, eLD_C_B);
    Exec(mem);

    std::cout << "Test 8Bit loads ...\n";
    fAF=0x1234;
    fBC=0xABCD;
    std::cout << "LD B A: ";
    mem->Write(this, fPC, eLD_B_A);
    Exec(mem);
    std::cout << (fBC==0x12CD?"OK\n":"!OK\n");
    std::cout << "LD C A: ";
    mem->Write(this, fPC, eLD_C_A);
    Exec(mem);
    std::cout << (fBC==0x1212?"OK\n":"!OK\n");

    std::cout << "ALU Increment: ";
    mem->Write(this, fPC, eINC_BC);
    Exec(mem);
    std::cout << (fBC==0x1213?"OK\n":"!OK\n");

    std::cout << "Inc B: ";
    fAF=0x00F0;
    mem->Write(this, fPC, eINC_B);
    Exec(mem);
    std::cout << (fAF==0x10 && fBC==0x1313?"OK\n":"!OK\n");

    std::cout << "Inc C: ";
    fAF=0x00F0;
    mem->Write(this, fPC, eINC_C);
    Exec(mem);
    std::cout << (fAF==0x10 && fBC==0x1314?"OK\n":"!OK\n");

    std::cout << "Dec C: ";
    fBC=0xFF00;
    mem->Write(this, fPC, eDEC_C);
    Exec(mem);
    std::cout << (fAF==0x50 && fBC==0xFFFF?"OK\n":"!OK\n");
  }

  bool XCPU::Interrupt(XMemory* mem,int inte)
  {
    #if defined DEBUG && defined VERBOSE
    std::cout << "LY_REG=" << (short)mem->fIOPorts[LY_REG] << ' ' << (short)mem->fInterruptRegister << ' ' << fIntFlag << '\n';
    #endif

    if((mem->fInterruptRegister&inte)&&fIntFlag)
    {
      if(fState==eHaltState)
      {
        fState=eRunState;
        fPC++;
      }
      mem->fIOPorts[IF_REG]&=~inte;
      fIntFlag=false;
    
      mem->Write(this, --fSP, Hi(fPC)>>8);
      mem->Write(this, --fSP, Lo(fPC));
  
      switch(inte)
      {
        case 1: fPC=0x40; break; //VBlank
        case 2: fPC=0x48; break; //LCDC
        case 4: fPC=0x50; break; //TimeOwfl
        case 8: fPC=0x58; break; //Serial
      }
      //std::cout << "PC1 = " << std::hex << fPC << std::dec << '\n';
      return true;
    }
    //std::cout << "PC2 = " << std::hex << fPC << std::dec << '\n';
    return false;
  }

  unsigned char XCPU::Exec(XMemory* mem)
  {
    unsigned short OpCode=mem->Read(fPC++);

    #if defined DEBUG && defined VERBOSE
    Display(mem, OpCode);
    #endif

    if(OpCode==0xCB)
    {
      OpCode<<=8;
      OpCode|=mem->Read(fPC++);
    }

    if(fEIFlag)
    {
      fIntFlag=1;
      fEIFlag=0;
    }

    //If in halt state didn't process opcode anymore
    if(fState==eHaltState) return 4;

    switch(OpCode)
    {
      //8Bit loads: LD nn, n : Put value nn into n
      case eLD_B_N:
        fBC=Lo(fBC)|((mem->Read(fPC++))<<8);
        return 8;
      case eLD_C_N:
        fBC=Hi(fBC)|(mem->Read(fPC++));
        return 8;
      case eLD_D_N:
        fDE=Lo(fDE)|((mem->Read(fPC++))<<8);
        return 8;
      case eLD_E_N:
        fDE=Hi(fDE)|(mem->Read(fPC++));
        return 8;
      case eLD_H_N:
        fHL=Lo(fHL)|((mem->Read(fPC++))<<8);
        return 8;
      case eLD_L_N:
        fHL=Hi(fHL)|(mem->Read(fPC++));
        return 8;

        //8Bit loads: LD r1,r2 : Put value r2 into r1
        //A reg
      case eLD_A_A:
        //NOP
        return 4;
      case eLD_A_B:
        fAF=Lo(fAF)|Hi(fBC);
        return 4;
      case eLD_A_C:
        fAF=Lo(fAF)|(Lo(fBC)<<8);
        return 4;
      case eLD_A_D:
        fAF=Lo(fAF)|Hi(fDE);
        return 4;
      case eLD_A_E:
        fAF=Lo(fAF)|(Lo(fDE)<<8);
        return 4;
      case eLD_A_H:
        fAF=Lo(fAF)|Hi(fHL);
        return 4;
      case eLD_A_L:
        fAF=Lo(fAF)|(Lo(fHL)<<8);
        return 4;
      case eLD_A_HL:
        fAF=Lo(fAF)|(mem->Read(fHL)<<8);
        return 8;

        //B reg
      case eLD_B_B:
        //NOP
        return 4;
      case eLD_B_C:
        fBC=Lo(fBC)|(Lo(fBC)<<8);
        return 4;
      case eLD_B_D:
        fBC=Lo(fBC)|Hi(fDE);
        return 4;
      case eLD_B_E:
        fBC=Lo(fBC)|(Lo(fDE)<<8);
        return 4;
      case eLD_B_H:
        fBC=Lo(fBC)|Hi(fHL);
        return 4;
      case eLD_B_L:
        fBC=Lo(fBC)|(Lo(fHL)<<8);
        return 4;
      case eLD_B_HL:
        fBC=Lo(fBC)|(mem->Read(fHL)<<8);
        return 8;

        //C reg
      case eLD_C_B:
        fBC=Hi(fBC)|(Hi(fBC)>>8);
        return 4;
      case eLD_C_C:
        //NOP
        return 4;
      case eLD_C_D:
        fBC=Hi(fBC)|(Hi(fDE)>>8);
        return 4;
      case eLD_C_E:
        fBC=Hi(fBC)|Lo(fDE);
        return 4;
      case eLD_C_H:
        fBC=Hi(fBC)|(Hi(fHL)>>8);
        return 4;
      case eLD_C_L:
        fBC=Hi(fBC)|Lo(fHL);
        return 4;
      case eLD_C_HL:
        fBC=Hi(fBC)|mem->Read(fHL);
        return 8;

        //D reg
      case eLD_D_B:
        fDE=Lo(fDE)|Hi(fBC);
        return 4;
      case eLD_D_C:
        fDE=Lo(fDE)|(Lo(fBC)<<8);
        return 4;
      case eLD_D_D:
        //NOP
        return 4;
      case eLD_D_E:
        fDE=Lo(fDE)|(Lo(fDE)<<8);
        return 4;
      case eLD_D_H:
        fDE=Lo(fDE)|Hi(fHL);
        return 4;
      case eLD_D_L:
        fDE=Lo(fDE)|(Lo(fHL)<<8);
        return 4;
      case eLD_D_HL:
        fDE=Lo(fDE)|(mem->Read(fHL)<<8);
        return 8;

        //E reg
      case eLD_E_B:
        fDE=Hi(fDE)|(Hi(fBC)>>8);
        return 4;
      case eLD_E_C:
        fDE=Hi(fDE)|Lo(fBC);
        return 4;
      case eLD_E_D:
        fDE=Hi(fDE)|(Hi(fDE)>>8);
        return 4;
      case eLD_E_E:
        //NOP
        return 4;
      case eLD_E_H:
        fDE=Hi(fDE)|(Hi(fHL)>>8);
        return 4;
      case eLD_E_L:
        fDE=Hi(fDE)|Lo(fHL);
        return 4;
      case eLD_E_HL:
        fDE=Hi(fDE)|mem->Read(fHL);
        return 8;

        //H reg
      case eLD_H_B:
        fHL=Lo(fHL)|Hi(fBC);
        return 4;
      case eLD_H_C:
        fHL=Lo(fHL)|(Lo(fBC)<<8);
        return 4;
      case eLD_H_D:
        fHL=Lo(fHL)|Hi(fDE);
        return 4;
      case eLD_H_E:
        fHL=Lo(fHL)|(Lo(fDE)<<8);
        return 4;
      case eLD_H_H:
        //NOP
        return 4;
      case eLD_H_L:
        fHL=Lo(fHL)|(Lo(fHL)<<8);
        return 4;
      case eLD_H_HL:
        fHL=Lo(fHL)|(mem->Read(fHL)<<8);
        return 8;

        //L reg
      case eLD_L_B:
        fHL=Hi(fHL)|(Hi(fBC)>>8);
        return 4;
      case eLD_L_C:
        fHL=Hi(fHL)|Lo(fBC);
        return 4;
      case eLD_L_D:
        fHL=Hi(fHL)|(Hi(fDE)>>8);
        return 4;
      case eLD_L_E:
        fHL=Hi(fHL)|Lo(fDE);
        return 4;
      case eLD_L_H:
        fHL=Hi(fHL)|(Hi(fHL)>>8);
        return 4;
      case eLD_L_L:
        //NOP
        return 4;
      case eLD_L_HL:
        fHL=Hi(fHL)|mem->Read(fHL);
        return 8;

        //HL Reg
      case eLD_HL_B:
        mem->Write(this, fHL,fBC>>8);
        return 8;
      case eLD_HL_C:
        mem->Write(this, fHL,Lo(fBC));
        return 8;
      case eLD_HL_D:
        mem->Write(this, fHL,fDE>>8);
        return 8;
      case eLD_HL_E:
        mem->Write(this, fHL,Lo(fDE));
        return 8;
      case eLD_HL_H:
        mem->Write(this, fHL,fHL>>8);
        return 8;
      case eLD_HL_L:
        mem->Write(this, fHL,Lo(fHL));
        return 8;
      case eLD_HL_N:
      {
        unsigned char Temp=mem->Read(fPC++);
        mem->Write(this, fHL,Temp);
      }
      return 12;

      //8Bit loads: LD A,n : put value n into A
      case eLD_A_BC:
        fAF=Lo(fAF)|(mem->Read(fBC)<<8);
        return 8;
      case eLD_A_DE:
        fAF=Lo(fAF)|(mem->Read(fDE)<<8);
        return 8;
      case eLD_A_NN:
      {
        unsigned short Temp=mem->ReadW(fPC);
        fPC+=2;
        fAF=Lo(fAF)|(mem->Read(Temp)<<8);
      }
      return 16;
      case eLD_A_SHARP:
      {
        unsigned char Temp=mem->Read(fPC++);
        fAF=Lo(fAF)|(Temp<<8);
      }
      return 8;

      //8Bit loads: LD n,A : put A into n
      case eLD_B_A:
        fBC=(fAF>>8)<<8|(Lo(fBC));
        return 4;
      case eLD_C_A:
        fBC=(fAF>>8)|(Hi(fBC));
        return 4;
      case eLD_D_A:
        fDE=(fAF>>8)<<8|(Lo(fDE));
        return 4;
      case eLD_E_A:
        fDE=(fAF>>8)|(Hi(fDE));
        return 4;
      case eLD_H_A:
        fHL=(fAF>>8)<<8|(Lo(fHL));
        return 4;
      case eLD_L_A:
        fHL=(fAF>>8)|(Hi(fHL));
        return 4;
      case eLD_BC_A:
        mem->Write(this, fBC,fAF>>8);
        return 8;
      case eLD_DE_A:
        mem->Write(this, fDE,fAF>>8);
        return 8;
      case eLD_HL_A:
        mem->Write(this, fHL,fAF>>8);
        return 8;
      case eLD_NN_A:
      {
        unsigned short Temp=mem->ReadW(fPC);
        fPC+=2;
        mem->Write(this, Temp, fAF>>8);
      }
      return 16;

      //8Bit loads: LD A,($FF00+C) : put value at address $FF00 + register
      case eLD_A_C2:
        fAF=Lo(fAF)|(mem->Read(0xFF00+Lo(fBC))<<8);
        return 8;

        //8Bit loads: LD ($FF00+C),A : put A into address $FF00 + register C.
      case eLD_C_A2:
        mem->Write(this, 0xFF00+Lo(fBC),Hi(fAF)>>8);
        return 8;

        //8Bit loads: Same as: LD A,(HL) - DEC HL
      case eLDD_A_HL:
                                 //LD A,(HL) + DEC HL
        fAF=Lo(fAF)|(mem->Read(fHL--)<<8);
        return 8;

        //8Bit loads: Same as: LD (HL),A - DEC HL
      case eLDD_HL_A:
        mem->Write(this, fHL--,fAF>>8);
        return 8;

        //8Bit loads: Same as: LD A,(HL) - INC HL
      case eLDI_A_HL:
        fAF=Lo(fAF)|(mem->Read(fHL++)<<8);
        return 8;

        //8Bit loads: Same as: LD (HL),A - INC HL
      case eLDI_HL_A:
        mem->Write(this, fHL++,fAF>>8);
        return 8;

        //8Bit loads: Put A into memory address $FF00+n.
      case eLDH_N_A:
        mem->Write(this, 0xFF00+mem->Read(fPC++), Hi(fAF)>>8);
        return 12;

        //8Bit loads: Put memory address $FF00+n into A.
      case eLDH_A_N:
        {
          unsigned char Temp=mem->Read(fPC++);
          fAF=(mem->Read(0xFF00+Temp)<<8)|Lo(fAF);
        }
        return 12;

      case eLD_BC_NN:            //16Bit loads: Put value NN into N
        fBC=mem->ReadW(fPC);
        fPC+=2;
        return 12;
      case eLD_DE_NN:
        fDE=mem->ReadW(fPC);
        fPC+=2;
        return 12;
      case eLD_HL_NN:
        fHL=mem->ReadW(fPC);
        fPC+=2;
        return 12;
      case eLD_SP_NN:
        fSP=mem->ReadW(fPC);
        fPC+=2;
        return 12;

        //16Bit loads: Put HL into SP
      case eLD_SP_HL:
        fSP=fHL;
        return 8;

        //16Bit loads: Put SP + n effective address into HL.
        /*
          Z - Reset.
          N - Reset.
          H - Set or reset according to operation.
          C - Set or reset according to operation.
          */
      case eLDHL_SP_N:
      {
        unsigned char Offset=mem->Read(fPC++);
        unsigned int Reg=fSP+Offset;
        fHL=Reg&0xFFFF;
                                 //Carry
        Reg&0x10000?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        ((fSP&0xF)+(Offset&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        UnSetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 12;

      //16Bit loads: Put Stack Pointer (SP) at address n.
      case eLD_NN_SP:
      {
        unsigned short Temp=mem->ReadW(fPC);
        fPC+=2;
        mem->Write(this, Temp, Lo(fSP));
        mem->Write(this, Temp+1, Hi(fSP)>>8);
      }
      return 20;

      //16Bit loads: Push register pair nn onto stack.
      //Decrement Stack Pointer (SP) twice.
      case ePUSH_AF:
        mem->Write(this, --fSP,Hi(fAF)>>8);
        mem->Write(this, --fSP,Lo(fAF));
        return 16;
      case ePUSH_BC:
        mem->Write(this, --fSP,Hi(fBC)>>8);
        mem->Write(this, --fSP,Lo(fBC));
        return 16;
      case ePUSH_DE:
        mem->Write(this, --fSP,Hi(fDE)>>8);
        mem->Write(this, --fSP,Lo(fDE));
        return 16;
      case ePUSH_HL:
        mem->Write(this, --fSP,Hi(fHL)>>8);
        mem->Write(this, --fSP,Lo(fHL));
        return 16;

        //16Bit load: Pop two bytes off stack into register pair nn.
        //Increment Stack Pointer (SP) twice.
      case ePOP_AF:
      {
        unsigned char Lo=mem->Read(fSP++);
        unsigned char Hi=mem->Read(fSP++);
        fAF=Hi<<8|Lo;
      }
      return 12;
      case ePOP_BC:
      {
        unsigned char Lo=mem->Read(fSP++);
        unsigned char Hi=mem->Read(fSP++);
        fBC=Hi<<8|Lo;
      }
      return 12;
      case ePOP_DE:
      {
        unsigned char Lo=mem->Read(fSP++);
        unsigned char Hi=mem->Read(fSP++);
        fDE=Hi<<8|Lo;
      }
      return 12;
      case ePOP_HL:
      {
        unsigned char Lo=mem->Read(fSP++);
        unsigned char Hi=mem->Read(fSP++);
        fHL=Hi<<8|Lo;
      }
      return 12;

      //8Bit ALU: Add n to A.
      // Z - Set if result is zero.
      //N - Reset.
      //H - Set if carry from bit 3.
      //C - Set if carry from bit 7.
      case eADD_A_A:
      {
        unsigned short Res=(Hi(fAF)>>8)<<1;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)<<1)>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_B:
      {
        unsigned short Res=(Hi(fAF)>>8)+(Hi(fBC)>>8);
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+((Hi(fBC)>>8)&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_C:
      {
        unsigned short Res=(Hi(fAF)>>8)+Lo(fBC);
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Lo(fBC)&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_D:
      {
        unsigned short Res=(Hi(fAF)>>8)+(Hi(fDE)>>8);
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+((Hi(fDE)>>8)&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_E:
      {
        unsigned short Res=(Hi(fAF)>>8)+Lo(fDE);
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Lo(fDE)&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_H:
      {
        unsigned short Res=(Hi(fAF)>>8)+(Hi(fHL)>>8);
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+((Hi(fHL)>>8)&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_L:
      {
        unsigned short Res=(Hi(fAF)>>8)+Lo(fHL);
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Lo(fHL)&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADD_A_HL:
      {
        unsigned char Temp=mem->Read(fHL);
        unsigned short Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 8;
      case eADD_A_SHARP:
      {
        unsigned char Temp=mem->Read(fPC++);
        unsigned short Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 8;

      /* 
              8Bit ALU: Add n + Carry flag to A.
      Z - Set if result is zero.
      N - Reset.
      H - Set if carry from bit 3.
      C - Set if carry from bit 7.
      */
      case eADC_A_A:
      {
        unsigned short Temp=(Hi(fAF)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_B:
      {
        unsigned short Temp=(Hi(fBC)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_C:
      {
        unsigned short Temp=Lo(fBC)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_D:
      {
        unsigned short Temp=(Hi(fDE)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_E:
      {
        unsigned short Temp=Lo(fDE)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_H:
      {
        unsigned short Temp=(Hi(fHL)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_L:
      {
        unsigned short Temp=Lo(fHL)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eADC_A_HL:
      {
        unsigned short Temp=mem->Read(fHL)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 8;
      case eADC_A_SHARP:
      {
        unsigned short Temp=mem->Read(fPC++)+(IsSetFlag(CFLAG)?1:0);
        unsigned int Res=(Hi(fAF)>>8)+Temp;
        Res&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (((Hi(fAF)>>8)&0xF)+(Temp&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Res&0xFF)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 8;

      /* 
      8Bit ALU: Subtract n from A.
      Z - Set if result is zero.
      N - Set.
      H - Set if no borrow from bit 4.
      C - Set if no borrow.
      */
      case eSUB_A:
        fAF=Lo(fAF);
        SetFlag(NFLAG);
        SetFlag(ZFLAG);          //A-A=0 ! Yes :)
        return 4;
      case eSUB_B:
      {
        unsigned char B=Hi(fBC)>>8;
        unsigned char A=Hi(fAF)>>8;
        B>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (B&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((B-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 4;
      case eSUB_C:
      {
        unsigned char C=Lo(fBC);
        unsigned char A=Hi(fAF)>>8;
        C>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (C&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((C-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 4;
      case eSUB_D:
      {
        unsigned char D=Hi(fDE)>>8;
        unsigned char A=Hi(fAF)>>8;
        D>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (D&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((D-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 4;
      case eSUB_E:
      {
        unsigned char E=Lo(fDE);
        unsigned char A=Hi(fAF)>>8;
        E>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (E&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((E-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 4;
      case eSUB_H:
      {
        unsigned char H=Hi(fHL)>>8;
        unsigned char A=Hi(fAF)>>8;
        H>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (H&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((H-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 4;
      case eSUB_L:
      {
        unsigned char L=Lo(fHL);
        unsigned char A=Hi(fAF)>>8;
        L>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (L&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((L-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 4;
      case eSUB_HL:
      {
        unsigned char Temp=mem->Read(fHL);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (Temp&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Temp-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 8;
      case eSUB_SHARP:
      {
        unsigned char Temp=mem->Read(fPC++);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
        (Temp&0xf)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((Temp-A)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
      }
      return 8;

      //8Bit ALU: Subtract n + Carry flag from A.
      /*
         Z - Set if result is zero.
         N - Set.
        H - Set if no borrow from bit 4.
        C - Set if no borrow.
      */
      case eSBC_A_A:
      {
        unsigned short Temp=(Hi(fAF)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_B:
      {
        unsigned short Temp=(Hi(fBC)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_C:
      {
        unsigned short Temp=Lo(fBC)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_D:
      {
        unsigned short Temp=(Hi(fDE)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_E:
      {
        unsigned short Temp=Lo(fDE)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_H:
      {
        unsigned short Temp=(Hi(fHL)>>8)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_L:
      {
        unsigned short Temp=Lo(fHL)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 4;
      case eSBC_A_HL:
      {
        unsigned short Temp=mem->Read(fHL)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 8;
      case eSBC_A_SHARP:
      {
        unsigned short Temp=mem->Read(fPC++)+(IsSetFlag(CFLAG)?1:0);
        unsigned char A=Hi(fAF)>>8;
        Temp>A?SetFlag(CFLAG):UnSetFlag(CFLAG);
                                 //Half carry
        (Temp&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fAF=Lo(fAF)|((A-Temp)<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
      }
      return 8;

      /* 
       8Bit ALU: Logically AND n with A, result in A.
      Z - Set if result is zero.
      N - Reset.
      H - Set.
      C - Reset.
      */
      case eAND_A:
        //A&A=A
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_B:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&(Hi(fBC)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_C:
        fAF=Lo(fAF)|((Hi(fAF)>>8)&(Lo(fBC)<<8));
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_D:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&(Hi(fDE)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_E:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&Lo(fDE))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_H:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&(Hi(fHL)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_L:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&Lo(fHL))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eAND_HL:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&mem->Read(fHL))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 8;
      case eAND_SHARP:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)&mem->Read(fPC++))<<8);
        #if defined DEBUG && defined VERBOSE
        std::cout << "AF=0x" << std::hex << fAF << std::dec << '\n';
        #endif
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 8;

        /* 
         8Bit ALU: Logical OR n with register A, result in A.
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Reset.
        */
      case eOR_A:
        //A|A=A
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_B:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|(Hi(fBC)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_C:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|Lo(fBC))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_D:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|(Hi(fDE)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_E:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|Lo(fDE))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_H:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|(Hi(fHL)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_L:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|Lo(fHL))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eOR_HL:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|mem->Read(fHL))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 8;
      case eOR_SHARP:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)|mem->Read(fPC++))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 8;

        /* 
                 8Bit ALU: Logical exclusive OR n with register A, result in A.
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Reset.
        */
      case eXOR_A:
        //A^A=0
        fAF=Lo(fAF);
        SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_B:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^(Hi(fBC)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_C:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^Lo(fBC))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_D:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^(Hi(fDE)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_E:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^Lo(fDE))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_H:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^(Hi(fHL)>>8))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_L:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^Lo(fHL))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eXOR_HL:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^mem->Read(fHL))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 8;
      case eXOR_SHARP:
        fAF=Lo(fAF)|(((Hi(fAF)>>8)^mem->Read(fPC++))<<8);
        Hi(fAF)>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 8;

        /* 
         8Bit ALU: Compare A with n. This is basically an A - n subtraction instruction but the results are thrown away.
        Z - Set if result is zero. (Set if A = n.)
        N - Set.
        H - Set if no borrow from bit 4.
        C - Set for no borrow. (Set if A < n.)
        */
      case eCP_A:
        //Believe me or not but A=A ... always
        SetFlag(ZFLAG);
        SetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        return 4;
      case eCP_B:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char B=Hi(fBC)>>8;
        unsigned short Temp=A-B;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (B&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 4;
      case eCP_C:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char C=Lo(fBC);
        unsigned short Temp=A-C;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (C&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 4;
      case eCP_D:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char D=Hi(fDE)>>8;
        unsigned short Temp=A-D;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (D&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 4;
      case eCP_E:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char E=Lo(fDE);
        unsigned short Temp=A-E;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (E&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 4;
      case eCP_H:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char H=Hi(fHL)>>8;
        unsigned short Temp=A-H;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (H&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 4;
      case eCP_L:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char L=Lo(fHL);
        unsigned short Temp=A-L;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (L&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 4;
      case eCP_HL:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char HL=mem->Read(fHL);
        unsigned short Temp=A-HL;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (HL&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 8;
      case eCP_SHARP:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char Res=mem->Read(fPC++);
        unsigned short Temp=A-Res;
        Temp&0xff?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        (Res&0xF)>(A&0xF)?SetFlag(HFLAG):UnSetFlag(HFLAG);
        Temp&0x100?SetFlag(CFLAG):UnSetFlag(CFLAG);
      }
      return 8;

      /* 
       8Bit ALU: Increment register n.
      Z - Set if result is zero.
      N - Reset.
      H - Set if carry from bit 3.
      C - Not affected.
      */
      case eINC_A:
        (fAF>>8)^0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fAF=(((fAF>>8)+1)<<8)|Lo(fAF);
        fAF>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_B:
        (fBC>>8)^0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fBC=(((fBC>>8)+1)<<8)|Lo(fBC);
        fBC>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_C:
        (fBC^0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fBC++;
        (fBC&0xFF)?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_D:
        (fDE>>8)^0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fDE=(((fDE>>8)+1)<<8)|Lo(fDE);
        fDE>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_E:
        (fDE^0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fDE++;
        (fDE&0xFF)?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_H:
        (fHL>>8)^0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fHL=(((fHL>>8)+1)<<8)|Lo(fHL);
        fHL>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_L:
        (fHL^0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fHL++;
        (fHL&0xFF)?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        return 4;
      case eINC_HL2:
      {
        unsigned char Temp=mem->Read(fHL);
        (Temp++^0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        (Temp)?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        mem->Write(this, fHL,Temp);
      }
      return 12;

      /* 
        8Bit ALU: Decrement register n.
      Z - Set if reselt is zero.
      N - Set.
      H - Set if no borrow from bit 4.
      C - Not affected.
      */
      case eDEC_A:
        (fAF>>8)&0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fAF=(((fAF>>8)-1)<<8)|Lo(fAF);
        fAF>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_B:
        (fBC>>8)&0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fBC=(((fBC>>8)-1)<<8)|Lo(fBC);
        fBC>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_C:
        (fBC&0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fBC--;
        Lo(fBC)?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_D:
        (fDE>>8)&0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fDE=(((fDE>>8)-1)<<8)|Lo(fDE);
        fDE>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_E:
        (fDE&0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fDE--;
        Lo(fDE)?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_H:
        (fHL>>8)&0x0F?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fHL=(((fHL>>8)-1)<<8)|Lo(fHL);
        fHL>>8?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_L:
        (fHL&0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        fHL--;
        (Lo(fHL))?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        return 4;
      case eDEC_HL2:
      {
        unsigned char Temp=mem->Read(fHL);
        (Temp&0xF)?UnSetFlag(HFLAG):SetFlag(HFLAG);
        Temp--;
        Temp?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        SetFlag(NFLAG);
        mem->Write(this, fHL,Temp);
      }
      return 12;

      /* 
               8Bit ALU: Add n to HL.
      Z - Not affected.
      N - Reset.
      H - Set if carry from bit 11.
      C - Set if carry from bit 15.
      */
      case eADD_HL_BC:
      {
        unsigned int Temp=fHL+fBC;
        UnSetFlag(NFLAG);
                                 //Carry
        Temp&0x10000?SetFlag(CFLAG):UnSetFlag(CFLAG);
        ((fHL&0xF)+(fBC&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fHL=Temp&0xFFFF;
      }
      return 8;
      case eADD_HL_DE:
      {
        unsigned int Temp=fHL+fDE;
        UnSetFlag(NFLAG);
                                 //Carry
        Temp&0x10000?SetFlag(CFLAG):UnSetFlag(CFLAG);
        ((fHL&0xF)+(fDE&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fHL=Temp&0xFFFF;
      }
      return 8;
      case eADD_HL_HL:
      {
        unsigned int Temp=fHL<<1;
        UnSetFlag(NFLAG);
                                 //Carry
        Temp&0x10000?SetFlag(CFLAG):UnSetFlag(CFLAG);
        ((fHL&0xF)<<1)>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fHL=Temp&0xFFFF;
      }
      return 8;
      case eADD_HL_SP:
      {
        unsigned int Temp=fHL+fSP;
        UnSetFlag(NFLAG);
                                 //Carry
        Temp&0x10000?SetFlag(CFLAG):UnSetFlag(CFLAG);
        ((fHL&0xF)+(fSP&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fHL=Temp&0xFFFF;
      }
      return 8;

      /* 
              8Bit ALU: Add n to Stack Pointer (SP).
      Z - Reset.
      N - Reset.
      H - Set or reset according to operation.
      C - Set or reset according to operation.
      */
      case eADD_SP_SHARP:
      {
        unsigned int Temp=fHL+mem->Read(fPC++);
        UnSetFlag(NFLAG);
                                 //Carry
        Temp&0x10000?SetFlag(CFLAG):UnSetFlag(CFLAG);
        ((fHL&0xF)+(fSP&0xF))>0xF?SetFlag(HFLAG):UnSetFlag(HFLAG);
        fHL=Temp&0xFFFF;
      }
      return 16;

      //8Bit ALU: Increment register nn.
      case eINC_BC:
        fBC++;
        return 8;
      case eINC_DE:
        fDE++;
        return 8;
      case eINC_HL:
        fHL++;
        return 8;
      case eINC_SP:
        fSP++;
        return 8;

        //8Bit ALU: Decrement reg nn.
      case eDEC_BC:
        fBC--;
        return 8;
      case eDEC_DE:
        fDE--;
        return 8;
      case eDEC_HL:
        fHL--;
        return 8;
      case eDEC_SP:
        fSP--;
        return 8;

        //Misc: Swap upper & lower nibles of n.
        /*
           Z - Set if result is zero.
           N - Reset.
           H - Reset.
           C - Reset.
        */
      case eSWAP_A:
      {
        unsigned char A=Hi(fAF)>>8;
        unsigned char SA=((A&0xF0)>>4)|((A&0x0F)<<4);
        SA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fAF=(SA<<8)|Lo(fAF);
      }
      return 8;
      case eSWAP_B:
      {
        unsigned char B=Hi(fBC)>>8;
        unsigned char SB=((B&0xF0)>>4)|((B&0x0F)<<4);
        SB?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fBC=(SB<<8)|Lo(fBC);
      }
      return 8;
      case eSWAP_C:
      {
        unsigned char C=Lo(fBC);
        unsigned char SC=((C&0xF0)>>4)|((C&0x0F)<<4);
        SC?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fBC=SC|Hi(fBC);
      }
      return 8;
      case eSWAP_D:
      {
        unsigned char D=Hi(fDE)>>8;
        unsigned char SD=((D&0xF0)>>4)|((D&0x0F)<<4);
        SD?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fDE=(SD<<8)|Lo(fDE);
      }
      return 8;
      case eSWAP_E:
      {
        unsigned char E=Lo(fDE);
        unsigned char SE=((E&0xF0)>>4)|((E&0x0F)<<4);
        SE?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fDE=SE|Hi(fDE);
      }
      return 8;
      case eSWAP_H:
      {
        unsigned char H=Hi(fHL)>>8;
        unsigned char SH=((H&0xF0)>>4)|((H&0x0F)<<4);
        SH?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fHL=(SH<<8)|Lo(fHL);
      }
      return 8;
      case eSWAP_L:
      {
        unsigned char L=Lo(fHL);
        unsigned char SL=((L&0xF0)>>4)|((L&0x0F)<<4);
        SL?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        fHL=SL|Hi(fHL);
      }
      return 8;
      case eSWAP_HL:
      {
        unsigned char Val=mem->Read(fHL);
        unsigned char SVal=((Val&0xF0)>>4)|((Val&0x0F)<<4);
        SVal?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        UnSetFlag(CFLAG);
        mem->Write(this,fHL,SVal);
      }
      return 16;

      /* 
              Misc: Decimal adjust register A. This instruction adjusts register A so that the correct representation of Binary Coded Decimal (BCD) is obtained.
      Z - Set if register A is zero.
      N - Not affected.
      H - Reset.
      C - Set or reset according to operation.
      */
      case eDAA:
      {
        unsigned short Temp=Hi(fAF)>>8;
        if((Temp&0xF)>0x9) Temp+=(IsSetFlag(NFLAG)?-1:1)*0x6;
        if((Temp&0xF0)>0x90) { Temp+=(IsSetFlag(NFLAG)?-1:1)*0x60; SetFlag(CFLAG); }
        else UnSetFlag(CFLAG);
        fAF=Lo(fAF)|((Temp&0xFF)<<8);
        UnSetFlag(HFLAG);
      }
      return 4;

      /* 
      Misc: Complement A register. (Flip all bits.)
      Z - Not affected.
      N - Set.
      H - Set.
      C - Not affected.
      */
      case eCPL:                 // CPL
        fAF=(~Hi(fAF)&0xFF00)|Lo(fAF);
        SetFlag(NFLAG);
        SetFlag(HFLAG);
        return 4;

        /* 
        Misc: Complement carry flag.
        If C flag is set, then reset it.
        If C flag is reset, then set it.
        Z - Not affected.
        N - Reset.
        H - Reset.
        C - Complemented.
        */
      case eCCF:
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        IsSetFlag(CFLAG)?UnSetFlag(CFLAG):SetFlag(CFLAG);
        return 4;

        /* 
                Misc: Set Carry Flag
                Z - Not affected.
                N - Reset.
                H - Reset.
                C - Set.
        */
      case eSCF:
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        SetFlag(CFLAG);
        return 4;

      case eNOP:                 //No operation
        return 4;

      case eHALT:                //Power down CPU until an interrupt occurs. Use this when ever possible to reduce energy consumption.
        if(fIntFlag)
        {
          fState=eHaltState;
          fPC--;
        }
        return 4;

      case eSTOP:                //Halt CPU & LCD display until button pressed.
        //Unsupported yet
        return 4;

      case eDI:                  //This instruction disables interrupts but not immediately. Interrupts are disabled after instruction after DI is executed.
        fIntFlag=false;
        fEIFlag=false;
        return 4;

      case eEI:                  //Enable interrupts. This intruction enables interrupts but not immediately. Interrupts are enabled after instruction after EI is  executed.
        fEIFlag=true;
        return 4;

        /* 
         Rotate & Shift
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
        */
      case eRLCA:                //Rotate A left. Old bit 7 to Carry flag.
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=A&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, true, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 4;
      case eRLA:                 //Rotate A left through Carry flag.
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=IsSetFlag(CFLAG);
        A&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, true, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 4;
      // C - Contains old bit 0 data.
      case eRRCA:                //Rotate A right. Old bit 0 to Carry flag.
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=A&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, false, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 4;
      case eRRA:                 //Rotate A right through Carry flag.
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=IsSetFlag(CFLAG);
        A&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, false, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 4;

      //  Extended OP code
      //        Rotate n left. Old bit 7 to Carry flag.
      case eRLC_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=A&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, true, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        bool Bit=B&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RB=Rotate(B, true, Bit);
        fBC=Lo(fBC)|(RB<<8);
        RB?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        bool Bit=C&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RC=Rotate(C, true, Bit);
        fBC=Hi(fBC)|RC;
        RC?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        bool Bit=D&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RD=Rotate(D, true, Bit);
        fDE=Lo(fDE)|(RD<<8);
        RD?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        bool Bit=E&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RE=Rotate(E, true, Bit);
        fDE=Hi(fDE)|RE;
        RE?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        bool Bit=H&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RH=Rotate(H, true, Bit);
        fHL=Lo(fHL)|(RH<<8);
        RH?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        bool Bit=L&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RL=Rotate(L, true, Bit);
        fHL=Hi(fHL)|RL;
        RL?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRLC_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fPC++);
        bool Bit=Val&0x80;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        mem->Write(this, fHL, (Val<<1)|(Val>>7));
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 16;

      // Rotate n left through Carry flag.
      case eRL_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=IsSetFlag(CFLAG);
        A&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, true, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        bool Bit=IsSetFlag(CFLAG);
        B&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RB=Rotate(B, true, Bit);
        fBC=Lo(fBC)|(RB<<8);
        RB?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        bool Bit=IsSetFlag(CFLAG);
        C&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RC=Rotate(C, true, Bit);
        fBC=Hi(fBC)|RC;
        RC?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        bool Bit=IsSetFlag(CFLAG);
        D&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RD=Rotate(D, true, Bit);
        fDE=Lo(fDE)|(RD<<8);
        RD?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        bool Bit=IsSetFlag(CFLAG);
        E&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RE=Rotate(E, true, Bit);
        fDE=Hi(fDE)|RE;
        RE?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        bool Bit=IsSetFlag(CFLAG);
        H&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RH=Rotate(H, true, Bit);
        fHL=Lo(fHL)|(RH<<8);
        RH?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        bool Bit=IsSetFlag(CFLAG);
        L&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RL=Rotate(L, true, Bit);
        fHL=Hi(fHL)|RL;
        RL?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRL_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fPC++);
        bool Bit=IsSetFlag(CFLAG);
        Val&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        Val=(Val<<1)|Bit;
        mem->Write(this, fHL, Val);
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }

      // Rotate n right. Old bit 0 to Carry flag.
      case eRRC_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=A&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, false, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        bool Bit=B&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RB=Rotate(B, false, Bit);
        fBC=Lo(fBC)|(RB<<8);
        RB?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        bool Bit=C&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RC=Rotate(C, false, Bit);
        fBC=Hi(fBC)|RC;
        RC?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        bool Bit=D&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RD=Rotate(D, false, Bit);
        fDE=Lo(fDE)|(RD<<8);
        RD?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        bool Bit=E&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RE=Rotate(E, false, Bit);
        fDE=Hi(fDE)|RE;
        RE?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        bool Bit=H&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RH=Rotate(H, false, Bit);
        fHL=Lo(fHL)|(RH<<8);
        RH?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        bool Bit=L&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RL=Rotate(L, false, Bit);
        fHL=Hi(fHL)|RL;
        RL?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRRC_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fPC++);
        bool Bit=Val&0x01;
        Bit?SetFlag(CFLAG):UnSetFlag(CFLAG);
        Val=(Val>>1)|(Bit<<7);
        mem->Write(this, fHL, Val);
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 16;

      // Rotate n right through Carry flag.
      case eRR_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        bool Bit=IsSetFlag(CFLAG);
        A&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RA=Rotate(A, false, Bit);
        fAF=Lo(fAF)|(RA<<8);
        RA?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        bool Bit=IsSetFlag(CFLAG);
        B&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RB=Rotate(B, false, Bit);
        fBC=Lo(fBC)|(RB<<8);
        RB?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        bool Bit=IsSetFlag(CFLAG);
        C&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RC=Rotate(C, false, Bit);
        fBC=Hi(fBC)|RC;
        RC?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        bool Bit=IsSetFlag(CFLAG);
        D&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RD=Rotate(D, false, Bit);
        fDE=Lo(fDE)|(RD<<8);
        RD?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        bool Bit=IsSetFlag(CFLAG);
        E&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RE=Rotate(E, false, Bit);
        fDE=Hi(fDE)|RE;
        RE?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        bool Bit=IsSetFlag(CFLAG);
        H&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RH=Rotate(H, false, Bit);
        fHL=Lo(fHL)|(RH<<8);
        RH?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        bool Bit=IsSetFlag(CFLAG);
        L&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char RL=Rotate(L, false, Bit);
        fHL=Hi(fHL)|RL;
        RL?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eRR_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fPC++);
        bool Bit=IsSetFlag(CFLAG);
        Val&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        Val=(Val>>1)|(Bit<<7);
        mem->Write(this, fHL, Val);
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 16;

      //Shift n left into Carry. LSB of n set to 0.
      case eSLA_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        A&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        A<<=1;
        fAF=Lo(fAF)|(A<<8);
        A?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        B&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        B<<=1;
        fBC=Lo(fBC)|(B<<8);
        B?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        C&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        C<<=1;
        fBC=Hi(fBC)|C;
        C?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        D&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        D<<=1;
        fDE=Lo(fDE)|(D<<8);
        D?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        E&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        E<<=1;
        fDE=Hi(fDE)|E;
        E?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        H&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        H<<=1;
        fHL=Lo(fHL)|(H<<8);
        H?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        L&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        L<<=1;
        fHL=Hi(fHL)|L;
        L?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSLA_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fHL);
        Val&0x80?SetFlag(CFLAG):UnSetFlag(CFLAG);
        Val<<=1;
        mem->Write(this, fHL, Val);
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 16;

      //Shift n right into Carry. MSB doesn't change.
      case eSRA_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        A&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=A&0x80;
        A>>=1;
        A|=Bit;                  //MSB doesn't change
        fAF=Lo(fAF)|(A<<8);
        A?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        B&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=B&0x80;
        B>>=1;
        B|=Bit;                  //MSB doesn't change
        fBC=Lo(fBC)|(B<<8);
        B?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        C&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=C&0x80;
        C>>=1;
        C|=Bit;                  //MSB doesn't change
        fBC=Hi(fBC)|C;
        C?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        D&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=D&0x80;
        D>>=1;
        D|=Bit;                  //MSB doesn't change
        fDE=Lo(fDE)|(D<<8);
        D?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        E&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=E&0x80;
        E>>=1;
        E|=Bit;                  //MSB doesn't change
        fDE=Hi(fDE)|E;
        E?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        H&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=H&0x80;
        H>>=1;
        H|=Bit;                  //MSB doesn't change
        fHL=Lo(fHL)|(H<<8);
        H?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        L&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=L&0x80;
        L>>=1;
        L|=Bit;                  //MSB doesn't change
        fHL=Hi(fHL)|L;
        L?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRA_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fHL);
        Val&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        unsigned char Bit=Val&0x80;
        Val>>=1;
        Val|=Bit;                //MSB doesn't change
        mem->Write(this, fHL, Val);
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 16;

      //Shift n right into Carry. MSB set to 0.
      case eSRL_A:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char A=Hi(fAF)>>8;
        A&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        A>>=1;
        A&=0x7F;                 //MSB set to 0
        fAF=Lo(fAF)|(A<<8);
        A?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_B:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char B=Hi(fBC)>>8;
        B&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        B>>=1;
        B&=0x7F;                 //MSB set to 0
        fBC=Lo(fBC)|(B<<8);
        B?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_C:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char C=Lo(fBC);
        C&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        C>>=1;
        C&=0x7F;                 //MSB set to 0
        fBC=Hi(fBC)|C;
        C?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_D:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char D=Hi(fDE)>>8;
        D&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        D>>=1;
        D&=0x7F;                 //MSB set to 0
        fDE=Lo(fDE)|(D<<8);
        D?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_E:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char E=Lo(fDE);
        E&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        E>>=1;
        E&=0x7F;                 //MSB set to 0
        fDE=Hi(fDE)|E;
        E?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_H:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char H=Hi(fHL)>>8;
        H&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        H>>=1;
        H&=0x7F;                 //MSB set to 0
        fHL=Lo(fHL)|(H<<8);
        H?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_L:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char L=Lo(fHL);
        L&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        L>>=1;
        L&=0x7F;                 //MSB set to 0
        fHL=Hi(fHL)|L;
        L?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 8;
      case eSRL_HL:
      {
        UnSetFlag(NFLAG);
        UnSetFlag(HFLAG);
        unsigned char Val=mem->Read(fHL);
        Val&0x01?SetFlag(CFLAG):UnSetFlag(CFLAG);
        Val>>=1;
        Val&=0x7F;               //MSB set to 0
        mem->Write(this, fHL, Val);
        Val?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
      }
      return 16;

      /* 
       Bit OpCodes
      Test bit b in register r.
      Z - Set if bit b of register r is 0.
      N - Reset.
      H - Set.
      C - Not affected.
      */
      //BIT 0
      case eBIT_0_A:
        Hi(fAF)&0x0100?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_B:
        Hi(fBC)&0x0100?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_C:
        Lo(fBC)&0x01?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_D:
        Hi(fDE)&0x0100?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_E:
        Lo(fDE)&0x01?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_H:
        Hi(fHL)&0x0100?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_L:
        Lo(fHL)&0x01?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_0_HL:
        mem->Read(fHL)&0x01?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 1
      case eBIT_1_A:
        Hi(fAF)&0x0200?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_B:
        Hi(fBC)&0x0200?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_C:
        Lo(fBC)&0x02?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_D:
        Hi(fDE)&0x0200?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_E:
        Lo(fDE)&0x02?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_H:
        Hi(fHL)&0x0200?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_L:
        Lo(fHL)&0x02?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_1_HL:
        mem->Read(fHL)&0x02?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 2
      case eBIT_2_A:
        Hi(fAF)&0x0400?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_B:
        Hi(fBC)&0x0400?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_C:
        Lo(fBC)&0x04?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_D:
        Hi(fDE)&0x0400?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_E:
        Lo(fDE)&0x04?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_H:
        Hi(fHL)&0x0400?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_L:
        Lo(fHL)&0x04?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_2_HL:
        mem->Read(fHL)&0x04?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 3
      case eBIT_3_A:
        Hi(fAF)&0x0800?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_B:
        Hi(fBC)&0x0800?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_C:
        Lo(fBC)&0x08?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_D:
        Hi(fDE)&0x0800?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_E:
        Lo(fDE)&0x08?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_H:
        Hi(fHL)&0x0800?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_L:
        Lo(fHL)&0x08?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_3_HL:
        mem->Read(fHL)&0x08?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 4
      case eBIT_4_A:
        Hi(fAF)&0x1000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_B:
        Hi(fBC)&0x1000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_C:
        Lo(fBC)&0x10?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_D:
        Hi(fDE)&0x1000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_E:
        Lo(fDE)&0x10?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_H:
        Hi(fHL)&0x1000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_L:
        Lo(fHL)&0x10?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_4_HL:
        mem->Read(fHL)&0x10?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 5
      case eBIT_5_A:
        Hi(fAF)&0x2000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_B:
        Hi(fBC)&0x2000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_C:
        Lo(fBC)&0x20?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_D:
        Hi(fDE)&0x2000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_E:
        Lo(fDE)&0x20?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_H:
        Hi(fHL)&0x2000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_L:
        Lo(fHL)&0x20?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_5_HL:
        mem->Read(fHL)&0x20?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 6
      case eBIT_6_A:
        Hi(fAF)&0x4000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_B:
        Hi(fBC)&0x4000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_C:
        Lo(fBC)&0x40?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_D:
        Hi(fDE)&0x4000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_E:
        Lo(fDE)&0x40?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_H:
        Hi(fHL)&0x4000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_L:
        Lo(fHL)&0x40?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_6_HL:
        mem->Read(fHL)&0x40?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;
        //BIT 7
      case eBIT_7_A:
        Hi(fAF)&0x8000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_B:
        Hi(fBC)&0x8000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_C:
        Lo(fBC)&0x80?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_D:
        Hi(fDE)&0x8000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_E:
        Lo(fDE)&0x80?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_H:
        Hi(fHL)&0x8000?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_L:
        Lo(fHL)&0x80?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 8;
      case eBIT_7_HL:
        mem->Read(fHL)&0x80?UnSetFlag(ZFLAG):SetFlag(ZFLAG);
        UnSetFlag(NFLAG);
        SetFlag(HFLAG);
        return 16;

        //Set bit b in register r.
        //BIT 0
      case eSET_0_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x0100);
        return 8;
      case eSET_0_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x0100);
        return 8;
      case eSET_0_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x01);
        return 8;
      case eSET_0_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x0100);
        return 8;
      case eSET_0_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x01);
        return 8;
      case eSET_0_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x0100);
        return 8;
      case eSET_0_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x01);
        return 8;
      case eSET_0_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x01);
      }
      return 16;
      //BIT 1
      case eSET_1_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x0200);
        return 8;
      case eSET_1_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x0200);
        return 8;
      case eSET_1_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x02);
        return 8;
      case eSET_1_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x0200);
        return 8;
      case eSET_1_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x02);
        return 8;
      case eSET_1_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x0200);
        return 8;
      case eSET_1_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x02);
        return 8;
      case eSET_1_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x02);
      }
      return 16;
      //BIT 2
      case eSET_2_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x0400);
        return 8;
      case eSET_2_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x0400);
        return 8;
      case eSET_2_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x04);
        return 8;
      case eSET_2_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x0400);
        return 8;
      case eSET_2_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x04);
        return 8;
      case eSET_2_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x0400);
        return 8;
      case eSET_2_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x04);
        return 8;
      case eSET_2_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x04);
      }
      return 16;
      //BIT 3
      case eSET_3_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x0800);
        return 8;
      case eSET_3_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x0800);
        return 8;
      case eSET_3_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x08);
        return 8;
      case eSET_3_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x0800);
        return 8;
      case eSET_3_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x08);
        return 8;
      case eSET_3_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x0800);
        return 8;
      case eSET_3_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x08);
        return 8;
      case eSET_3_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x08);
      }
      return 16;
      //BIT 4
      case eSET_4_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x1000);
        return 8;
      case eSET_4_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x1000);
        return 8;
      case eSET_4_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x10);
        return 8;
      case eSET_4_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x1000);
        return 8;
      case eSET_4_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x10);
        return 8;
      case eSET_4_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x1000);
        return 8;
      case eSET_4_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x10);
        return 8;
      case eSET_4_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x10);
      }
      return 16;
      //BIT 5
      case eSET_5_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x2000);
        return 8;
      case eSET_5_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x2000);
        return 8;
      case eSET_5_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x20);
        return 8;
      case eSET_5_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x2000);
        return 8;
      case eSET_5_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x20);
        return 8;
      case eSET_5_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x2000);
        return 8;
      case eSET_5_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x20);
        return 8;
      case eSET_5_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x20);
      }
      return 16;
      //BIT 6
      case eSET_6_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x4000);
        return 8;
      case eSET_6_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x4000);
        return 8;
      case eSET_6_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x40);
        return 8;
      case eSET_6_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x4000);
        return 8;
      case eSET_6_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x40);
        return 8;
      case eSET_6_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x4000);
        return 8;
      case eSET_6_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x40);
        return 8;
      case eSET_6_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x40);
      }
      return 16;
      //BIT 7
      case eSET_7_A:
        fAF=Lo(fAF)|(Hi(fAF)|0x8000);
        return 8;
      case eSET_7_B:
        fBC=Lo(fBC)|(Hi(fBC)|0x8000);
        return 8;
      case eSET_7_C:
        fBC=Hi(fBC)|(Lo(fBC)|0x80);
        return 8;
      case eSET_7_D:
        fDE=Lo(fDE)|(Hi(fDE)|0x8000);
        return 8;
      case eSET_7_E:
        fDE=Hi(fDE)|(Lo(fDE)|0x80);
        return 8;
      case eSET_7_H:
        fHL=Lo(fHL)|(Hi(fHL)|0x8000);
        return 8;
      case eSET_7_L:
        fHL=Hi(fHL)|(Lo(fHL)|0x80);
        return 8;
      case eSET_7_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res|0x80);
      }
      return 16;

      //Reset bit b in register r.
      //BIT 0
      case eRES_0_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xFE00);
        return 8;
      case eRES_0_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xFE00);
        return 8;
      case eRES_0_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xFE);
        return 8;
      case eRES_0_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xFE00);
        return 8;
      case eRES_0_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xFE);
        return 8;
      case eRES_0_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xFE00);
        return 8;
      case eRES_0_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xFE);
        return 8;
      case eRES_0_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xFE);
      }
      return 16;
      //BIT 1
      case eRES_1_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xFD00);
        return 8;
      case eRES_1_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xFD00);
        return 8;
      case eRES_1_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xFD);
        return 8;
      case eRES_1_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xFD00);
        return 8;
      case eRES_1_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xFD);
        return 8;
      case eRES_1_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xFD00);
        return 8;
      case eRES_1_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xFD);
        return 8;
      case eRES_1_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xFD);
      }
      return 16;
      //BIT 2
      case eRES_2_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xFB00);
        return 8;
      case eRES_2_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xFB00);
        return 8;
      case eRES_2_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xFB);
        return 8;
      case eRES_2_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xFB00);
        return 8;
      case eRES_2_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xFB);
        return 8;
      case eRES_2_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xFB00);
        return 8;
      case eRES_2_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xFB);
        return 8;
      case eRES_2_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xFB);
      }
      return 16;
      //BIT 3
      case eRES_3_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xF700);
        return 8;
      case eRES_3_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xF700);
        return 8;
      case eRES_3_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xF7);
        return 8;
      case eRES_3_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xF700);
        return 8;
      case eRES_3_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xF7);
        return 8;
      case eRES_3_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xF700);
        return 8;
      case eRES_3_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xF7);
        return 8;
      case eRES_3_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xF7);
      }
      return 16;
      //BIT 4
      case eRES_4_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xEF00);
        return 8;
      case eRES_4_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xEF00);
        return 8;
      case eRES_4_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xEF);
        return 8;
      case eRES_4_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xEF00);
        return 8;
      case eRES_4_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xEF);
        return 8;
      case eRES_4_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xEF00);
        return 8;
      case eRES_4_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xEF);
        return 8;
      case eRES_4_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xEF);
      }
      return 16;
      //BIT 5
      case eRES_5_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xDF00);
        return 8;
      case eRES_5_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xDF00);
        return 8;
      case eRES_5_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xDF);
        return 8;
      case eRES_5_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xDF00);
        return 8;
      case eRES_5_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xDF);
        return 8;
      case eRES_5_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xDF00);
        return 8;
      case eRES_5_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xDF);
        return 8;
      case eRES_5_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xDF);
      }
      return 16;
      //BIT 6
      case eRES_6_A:
        fAF=Lo(fAF)|(Hi(fAF)&0xBF00);
        return 8;
      case eRES_6_B:
        fBC=Lo(fBC)|(Hi(fBC)&0xBF00);
        return 8;
      case eRES_6_C:
        fBC=Hi(fBC)|(Lo(fBC)&0xBF);
        return 8;
      case eRES_6_D:
        fDE=Lo(fDE)|(Hi(fDE)&0xBF00);
        return 8;
      case eRES_6_E:
        fDE=Hi(fDE)|(Lo(fDE)&0xBF);
        return 8;
      case eRES_6_H:
        fHL=Lo(fHL)|(Hi(fHL)&0xBF00);
        return 8;
      case eRES_6_L:
        fHL=Hi(fHL)|(Lo(fHL)&0xBF);
        return 8;
      case eRES_6_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0xBF);
      }
      return 16;
      //BIT 7
      case eRES_7_A:
        fAF=Lo(fAF)|(Hi(fAF)&0x7F00);
        return 8;
      case eRES_7_B:
        fBC=Lo(fBC)|(Hi(fBC)&0x7F00);
        return 8;
      case eRES_7_C:
        fBC=Hi(fBC)|(Lo(fBC)&0x7F);
        return 8;
      case eRES_7_D:
        fDE=Lo(fDE)|(Hi(fDE)&0x7F00);
        return 8;
      case eRES_7_E:
        fDE=Hi(fDE)|(Lo(fDE)&0x7F);
        return 8;
      case eRES_7_H:
        fHL=Lo(fHL)|(Hi(fHL)&0x7F00);
        return 8;
      case eRES_7_L:
        fHL=Hi(fHL)|(Lo(fHL)&0x7F);
        return 8;
      case eRES_7_HL:
      {
        unsigned char Res=mem->Read(fHL);
        mem->Write(this, fHL, Res&0x7F);
      }
      return 16;

      //Jumps
      //Jump to address nn.
      case eJP_NN:
        fPC=mem->ReadW(fPC);
        return 16;

        /* 
        Jump to address n if following condition is true:
        cc = NZ, Jump if Z flag is reset.
        cc = Z, Jump if  Z flag is set.
        cc = NC, Jump if C flag is reset.
        cc = C, Jump if  C flag is set.
        */
      case eJP_NZ_NN:
        if(IsSetFlag(ZFLAG))
        {
          fPC+=2;
          return 12;
        }
        else
        {
          fPC=mem->ReadW(fPC);
          return 16;
        }
      case eJP_Z_NN:
        if(IsSetFlag(ZFLAG))
        {
          fPC=mem->ReadW(fPC);
          return 16;
        }
        else
        {
          fPC+=2;
          return 12;
        }
      case eJP_NC_NN:
        if(IsSetFlag(CFLAG))
        {
          fPC+=2;
          return 12;
        }
        else
        {
          fPC=mem->ReadW(fPC);
                                 //fClockTicks++;
          return 16;
        }
      case eJP_C_NN:
        if(IsSetFlag(CFLAG))
        {
          fPC=mem->ReadW(fPC);
          return 16;
        }
        else
        {
          fPC+=2;
          return 12;
        }

        //Jump to address contained in HL.
      case eJP_HL:
        fPC=fHL;
        return 4;

        //Add n to current address and jump to it.
      case eJP_N:
        fPC+=(char)mem->Read(fPC)+1;
        return 12;

        /* 
        If following condition is true then add n to current address and jump to it:
        Use with:
        n = one byte signed immediate value
        cc = NZ, Jump if Z flag is reset.
        cc = Z, Jump if Z  flag is set.
        cc = NC, Jump if C flag is reset.
        cc = C, Jump if C  flag is set.
        */
      case eJR_NZ_SHARP:
        if(IsSetFlag(ZFLAG))
        {
          fPC++;
          return 8;
        }
        else
        {
          fPC+=((char)mem->Read(fPC++))+1;
          //fClocksTicks++;
          return 12;
        }
      case eJR_Z_SHARP:
        if(IsSetFlag(ZFLAG))
        {
          fPC+=(char)mem->Read(fPC)+1;
          //fClocksTicks++;
          return 12;
        }
        else
        {
          fPC++;
          return 8;
        }
      case eJR_NC_SHARP:
        if(IsSetFlag(CFLAG))
        {
          fPC++;
          return 8;
        }
        else
        {
          fPC+=(char)mem->Read(fPC)+1;
          //fClocksTicks++;
          return 12;
        }
      case eJR_C_SHARP:
        if(IsSetFlag(CFLAG))
        {
          fPC+=(char)mem->Read(fPC)+1; 
          //fClocksTicks++;
          return 24;
        }
        else
        {
          fPC++;
          return 12;
        }

        /* 
        Call
        Push address of next instruction onto stack and then jump to address nn. Use with:
        nn = two byte immediate value. (LS byte first.)
        */
      case eCALL_NN:
      {
        unsigned char Low=mem->Read(fPC++);
        unsigned char High=mem->Read(fPC++);
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=High<<8|Low;
      }
      return 24;

      /* Call address n if following condition is true:
          cc = NZ, Call if Z flag is reset.
          cc = Z, Call  if Z flag is set.
          cc = NC, Call if C flag is reset.
          cc = C, Call  if C flag is set.
          */
      case eCALL_NZ_NN:
        if(IsSetFlag(ZFLAG))
        {
          fPC+=2;
          return 12;
        }
        else
        {
          unsigned char Low=mem->Read(fPC++);
          unsigned char High=mem->Read(fPC++);
          mem->Write(this, fSP--, Hi(fPC)>>8);
          mem->Write(this, fSP--, Lo(fPC));
          fPC=High<<8|Low;
          //fClockTocks+=3;
          return 24;
        }
      case eCALL_Z_NN:
        if(IsSetFlag(ZFLAG))
        {
          unsigned char Low=mem->Read(fPC++);
          unsigned char High=mem->Read(fPC++);
          mem->Write(this, fSP--, Hi(fPC)>>8);
          mem->Write(this, fSP--, Lo(fPC));
          fPC=High<<8|Low;
                                 //fClockTocks+=3;
          return 24;
        }
        else
        {
          fPC+=2;
          return 12;
        }
      case eCALL_NC_NN:
        if(IsSetFlag(CFLAG))
        {
          fPC+=2;
          return 12;
        }
        else
        {
          unsigned char Low=mem->Read(fPC++);
          unsigned char High=mem->Read(fPC++);
          mem->Write(this, fSP--, Hi(fPC)>>8);
          mem->Write(this, fSP--, Lo(fPC));
          fPC=High<<8|Low;
                                 //fClockTocks+=3;
          return 24;
        }
      case eCALL_C_NN:
        if(IsSetFlag(CFLAG))
        {
          unsigned char Low=mem->Read(fPC++);
          unsigned char High=mem->Read(fPC++);
          mem->Write(this, fSP--, Hi(fPC)>>8);
          mem->Write(this, fSP--, Lo(fPC));
          fPC=High<<8|Low;
                                 //fClockTocks+=3;
          return 24;
        }
        else
        {
          fPC+=2;
          return 12;
        }

        //Restarts
        // Push present address onto stack.
        // Jump to address $0000 + n.
      case eRST_00H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0;
        return 16;
      case eRST_08H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x8;
        return 16;
      case eRST_10H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x10;
        return 16;
      case eRST_18H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x18;
        return 16;
      case eRST_20H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x20;
        return 16;
      case eRST_28H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x28;
        return 16;
      case eRST_30H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x30;
        return 16;
      case eRST_38H:
        mem->Write(this, --fSP, Hi(fPC)>>8);
        mem->Write(this, --fSP, Lo(fPC));
        fPC=0x38;
        return 16;

        //Return
        //Pop two bytes from stack & jump to that address.
      case eRET:
        {
          unsigned char Lo=mem->Read(fSP++);
          unsigned char Hi=mem->Read(fSP++);
          fPC=(Hi<<8)|Lo;
        }
        return 16;

        /*
         Return if following condition is true:
         Use  with:
           cc = NZ, Return if Z flag is reset.
           cc = Z,  Return if Z flag is set.
           cc = NC, Return if C flag is reset.
           cc = C,  Return if C flag is set.
           */
      case eRET_NZ:
        if(!IsSetFlag(ZFLAG))
        {
          fPC=0|mem->Read(fSP++);
          fPC|=mem->Read(fSP++)<<8;
                                 //fClockTicks+=3;
          return 20;
        }
        return 8;
      case eRET_Z:
        if(IsSetFlag(ZFLAG))
        {
          fPC=0|mem->Read(fSP++);
          fPC|=mem->Read(fSP++)<<8;
                                 //fClockTicks+=3;
          return 20;
        }
        return 8;
      case eRET_NC:
        if(!IsSetFlag(CFLAG))
        {
          fPC=0|mem->Read(fSP++);
          fPC|=mem->Read(fSP++)<<8;
                                 //fClockTicks+=3;
          return 20;
        }
        return 8;
      case eRET_C:
        if(IsSetFlag(CFLAG))
        {
          fPC=0|mem->Read(fSP++);
          fPC|=mem->Read(fSP++)<<8;
                                 //fClockTicks+=3;
          return 20;
        }
        return 8;

        //Pop two bytes from stack & jump to that address then enable interrupts.
      case eRETI:
        fPC=0|mem->Read(fSP++);
        fPC|=mem->Read(fSP++)<<8;
        fIntFlag=true;
        return 16;

      default:
        std::cout << "[WARN] Unknown OP code " << std::hex << "0x" << (unsigned short)OpCode << "\n";
        return 0;
    }
  }

  unsigned char XCPU::Rotate(unsigned char reg, bool left, bool bit)
  {
    return left?((reg<<1)|(bit?1:0)):((reg>>1)|(bit?0x80:0));
  }
}
