/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __CPU_H__
#define __CPU_H__

#include "memory.h"

#define Hi(v) (v&0xFF00)
#define Lo(v) (v&0x00FF)

//Flag register
#define ZFLAG 0x80               //Zero flag
#define NFLAG 0x40               //Substract flag
#define HFLAG 0x20               //Half carry flag
#define CFLAG 0x10               //Carry flag

//Interruptions
#define VBLANK_INT 0x01
#define LCDC_INT 0x02
#define TIMEOWFL 0x04
#define SERIAL 0x08

#define URegister unsigned short

/*!
 * @brief Hardware of GB
 * @details
 */
namespace Hardware
{
  class XCPU
  {
    private:
      enum ESpeed
      {
        eSimple,
        eDouble
      };

      enum EStates
      {
        eRunState=0,
        eHaltState=1,
        eStopState=2
      };

      enum EOpCodes
      {
        eNOP=0x00,
        eLD_BC_NN=0x01,
        eLD_BC_A=0x02,
        eINC_BC=0x03,
        eINC_B=0x04,
        eDEC_B=0x05,
        eLD_B_N=0x06,
        eRLCA=0x07,
        eLD_NN_SP=0x08,
        eADD_HL_BC=0x09,
        eLD_A_BC=0x0A,
        eDEC_BC=0x0B,
        eINC_C=0x0C,
        eDEC_C=0x0D,
        eLD_C_N=0x0E,
        eRRCA=0x0F,
        eSTOP=0x10,
        eLD_DE_NN=0x11,
        eLD_DE_A=0x12,
        eINC_DE=0x13,
        eINC_D=0x14,
        eDEC_D=0x15,
        eLD_D_N=0x16,
        eRLA=0x17,
        eJP_N=0x18,
        eADD_HL_DE=0x19,
        eLD_A_DE=0x1A,
        eDEC_DE=0x1B,
        eINC_E=0x1C,
        eDEC_E=0x1D,
        eLD_E_N=0x1E,
        eRRA=0x1F,
        eJR_NZ_SHARP=0x20,
        eLD_HL_NN=0x21,
        eLDI_HL_A=0x22,
        eINC_HL=0x23,
        eINC_H=0x24,
        eDEC_H=0x25,
        eLD_H_N=0x26,
        eDAA=0x27,
        eJR_Z_SHARP=0x28,
        eADD_HL_HL=0x29,
        eLDI_A_HL=0x2A,
        eDEC_HL=0x2B,
        eINC_L=0x2C,
        eDEC_L=0x2D,
        eLD_L_N=0x2E,
        eCPL=0x2F,
        eJR_NC_SHARP=0x30,
        eLD_SP_NN=0x31,
        eLDD_HL_A=0x32,
        eINC_SP=0x33,
        eINC_HL2=0x34,
        eDEC_HL2=0x35,
        eLD_HL_N=0x36,
        eSCF=0x37,
        eJR_C_SHARP=0x38,
        eADD_HL_SP=0x39,
        eLDD_A_HL=0x3A,
        eDEC_SP=0x3B,
        eINC_A=0x3C,
        eDEC_A=0x3D,
        eLD_A_SHARP=0x3E,
        eCCF=0x3F,
        eLD_B_B=0x40,
        eLD_B_C=0x41,
        eLD_B_D=0x42,
        eLD_B_E=0x43,
        eLD_B_H=0x44,
        eLD_B_L=0x45,
        eLD_B_HL=0x46,
        eLD_B_A=0x47,
        eLD_C_B=0x48,
        eLD_C_C=0x49,
        eLD_C_D=0x4A,
        eLD_C_E=0x4B,
        eLD_C_H=0x4C,
        eLD_C_L=0x4D,
        eLD_C_HL=0x4E,
        eLD_C_A=0x4F,
        eLD_D_B=0x50,
        eLD_D_C=0x51,
        eLD_D_D=0x52,
        eLD_D_E=0x53,
        eLD_D_H=0x54,
        eLD_D_L=0x55,
        eLD_D_HL=0x56,
        eLD_D_A=0x57,
        eLD_E_B=0x58,
        eLD_E_C=0x59,
        eLD_E_D=0x5A,
        eLD_E_E=0x5B,
        eLD_E_H=0x5C,
        eLD_E_L=0x5D,
        eLD_E_HL=0x5E,
        eLD_E_A=0x5F,
        eLD_H_B=0x60,
        eLD_H_C=0x61,
        eLD_H_D=0x62,
        eLD_H_E=0x63,
        eLD_H_H=0x64,
        eLD_H_L=0x65,
        eLD_H_HL=0x66,
        eLD_H_A=0x67,
        eLD_L_B=0x68,
        eLD_L_C=0x69,
        eLD_L_D=0x6A,
        eLD_L_E=0x6B,
        eLD_L_H=0x6C,
        eLD_L_L=0x6D,
        eLD_L_HL=0x6E,
        eLD_L_A=0x6F,
        eLD_HL_B=0x70,
        eLD_HL_C=0x71,
        eLD_HL_D=0x72,
        eLD_HL_E=0x73,
        eLD_HL_H=0x74,
        eLD_HL_L=0x75,
        eHALT=0x76,
        eLD_HL_A=0x77,
        eLD_A_B=0x78,
        eLD_A_C=0x79,
        eLD_A_D=0x7A,
        eLD_A_E=0x7B,
        eLD_A_H=0x7C,
        eLD_A_L=0x7D,
        eLD_A_HL=0x7E,
        eLD_A_A=0x7F,
        eADD_A_B=0x80,
        eADD_A_C=0x81,
        eADD_A_D=0x82,
        eADD_A_E=0x83,
        eADD_A_H=0x84,
        eADD_A_L=0x85,
        eADD_A_HL=0x86,
        eADD_A_A=0x87,
        eADC_A_B=0x88,
        eADC_A_C=0x89,
        eADC_A_D=0x8A,
        eADC_A_E=0x8B,
        eADC_A_H=0x8C,
        eADC_A_L=0x8D,
        eADC_A_HL=0x8E,
        eADC_A_A=0x8F,
        eSUB_B=0x90,
        eSUB_C=0x91,
        eSUB_D=0x92,
        eSUB_E=0x93,
        eSUB_H=0x94,
        eSUB_L=0x95,
        eSUB_HL=0x96,
        eSUB_A=0x97,
        eSBC_A_B=0x98,
        eSBC_A_C=0x99,
        eSBC_A_D=0x9A,
        eSBC_A_E=0x9B,
        eSBC_A_H=0x9C,
        eSBC_A_L=0x9D,
        eSBC_A_HL=0x9E,
        eSBC_A_A=0x9F,
        eAND_B=0xA0,
        eAND_C=0xA1,
        eAND_D=0xA2,
        eAND_E=0xA3,
        eAND_H=0xA4,
        eAND_L=0xA5,
        eAND_HL=0xA6,
        eAND_A=0xA7,
        eXOR_B=0xA8,
        eXOR_C=0xA9,
        eXOR_D=0xAA,
        eXOR_E=0xAB,
        eXOR_H=0xAC,
        eXOR_L=0xAD,
        eXOR_HL=0xAE,
        eXOR_A=0xAF,
        eOR_B=0xB0,
        eOR_C=0xB1,
        eOR_D=0xB2,
        eOR_E=0xB3,
        eOR_H=0xB4,
        eOR_L=0xB5,
        eOR_HL=0xB6,
        eOR_A=0xB7,
        eCP_B=0xB8,
        eCP_C=0xB9,
        eCP_D=0xBA,
        eCP_E=0xBB,
        eCP_H=0xBC,
        eCP_L=0xBD,
        eCP_HL=0xBE,
        eCP_A=0xBF,
        eRET_NZ=0xC0,
        ePOP_BC=0xC1,
        eJP_NZ_NN=0xC2,
        eJP_NN=0xC3,
        eCALL_NZ_NN=0xC4,
        ePUSH_BC=0xC5,
        eADD_A_SHARP=0xC6,
        eRST_00H=0xC7,
        eRET_Z=0xC8,
        eRET=0xC9,
        eJP_Z_NN=0xCA,

        eRLC_B=0xCB00,
        eRLC_C=0xCB01,
        eRLC_D=0xCB02,
        eRLC_E=0xCB03,
        eRLC_H=0xCB04,
        eRLC_L=0xCB05,
        eRLC_HL=0xCB06,
        eRLC_A=0xCB07,
        eRRC_B=0xCB08,
        eRRC_C=0xCB09,
        eRRC_D=0xCB0A,
        eRRC_E=0xCB0B,
        eRRC_H=0xCB0C,
        eRRC_L=0xCB0D,
        eRRC_HL=0xCB0E,
        eRRC_A=0xCB0F,
        eRL_B=0xCB10,
        eRL_C=0xCB11,
        eRL_D=0xCB12,
        eRL_E=0xCB13,
        eRL_H=0xCB14,
        eRL_L=0xCB15,
        eRL_HL=0xCB16,
        eRL_A=0xCB17,
        eRR_B=0xCB18,
        eRR_C=0xCB19,
        eRR_D=0xCB1A,
        eRR_E=0xCB1B,
        eRR_H=0xCB1C,
        eRR_L=0xCB1D,
        eRR_HL=0xCB1E,
        eRR_A=0xCB1F,
        eSLA_B=0xCB20,
        eSLA_C=0xCB21,
        eSLA_D=0xCB22,
        eSLA_E=0xCB23,
        eSLA_H=0xCB24,
        eSLA_L=0xCB25,
        eSLA_HL=0xCB26,
        eSLA_A=0xCB27,
        eSRA_B=0xCB28,
        eSRA_C=0xCB29,
        eSRA_D=0xCB2A,
        eSRA_E=0xCB2B,
        eSRA_H=0xCB2C,
        eSRA_L=0xCB2D,
        eSRA_HL=0xCB2E,
        eSRA_A=0xCB2F,
        eSWAP_B=0xCB30,
        eSWAP_C=0xCB31,
        eSWAP_D=0xCB32,
        eSWAP_E=0xCB33,
        eSWAP_H=0xCB34,
        eSWAP_L=0xCB35,
        eSWAP_HL=0xCB36,
        eSWAP_A=0xCB37,
        eSRL_B=0xCB38,
        eSRL_C=0xCB39,
        eSRL_D=0xCB3A,
        eSRL_E=0xCB3B,
        eSRL_H=0xCB3C,
        eSRL_L=0xCB3D,
        eSRL_HL=0xCB3E,
        eSRL_A=0xCB3F,
        eBIT_0_B=0xCB40,
        eBIT_0_C=0xCB41,
        eBIT_0_D=0xCB42,
        eBIT_0_E=0xCB43,
        eBIT_0_H=0xCB44,
        eBIT_0_L=0xCB45,
        eBIT_0_HL=0xCB46,
        eBIT_0_A=0xCB47,
        eBIT_1_B=0xCB48,
        eBIT_1_C=0xCB49,
        eBIT_1_D=0xCB4A,
        eBIT_1_E=0xCB4B,
        eBIT_1_H=0xCB4C,
        eBIT_1_L=0xCB4D,
        eBIT_1_HL=0xCB4E,
        eBIT_1_A=0xCB4F,
        eBIT_2_B=0xCB50,
        eBIT_2_C=0xCB51,
        eBIT_2_D=0xCB52,
        eBIT_2_E=0xCB53,
        eBIT_2_H=0xCB54,
        eBIT_2_L=0xCB55,
        eBIT_2_HL=0xCB56,
        eBIT_2_A=0xCB57,
        eBIT_3_B=0xCB58,
        eBIT_3_C=0xCB59,
        eBIT_3_D=0xCB5A,
        eBIT_3_E=0xCB5B,
        eBIT_3_H=0xCB5C,
        eBIT_3_L=0xCB5D,
        eBIT_3_HL=0xCB5E,
        eBIT_3_A=0xCB5F,
        eBIT_4_B=0xCB60,
        eBIT_4_C=0xCB61,
        eBIT_4_D=0xCB62,
        eBIT_4_E=0xCB63,
        eBIT_4_H=0xCB64,
        eBIT_4_L=0xCB65,
        eBIT_4_HL=0xCB66,
        eBIT_4_A=0xCB67,
        eBIT_5_B=0xCB68,
        eBIT_5_C=0xCB69,
        eBIT_5_D=0xCB6A,
        eBIT_5_E=0xCB6B,
        eBIT_5_H=0xCB6C,
        eBIT_5_L=0xCB6D,
        eBIT_5_HL=0xCB6E,
        eBIT_5_A=0xCB6F,
        eBIT_6_B=0xCB70,
        eBIT_6_C=0xCB71,
        eBIT_6_D=0xCB72,
        eBIT_6_E=0xCB73,
        eBIT_6_H=0xCB74,
        eBIT_6_L=0xCB75,
        eBIT_6_HL=0xCB76,
        eBIT_6_A=0xCB77,
        eBIT_7_B=0xCB78,
        eBIT_7_C=0xCB79,
        eBIT_7_D=0xCB7A,
        eBIT_7_E=0xCB7B,
        eBIT_7_H=0xCB7C,
        eBIT_7_L=0xCB7D,
        eBIT_7_HL=0xCB7E,
        eBIT_7_A=0xCB7F,
        eRES_0_B=0xCB80,
        eRES_0_C=0xCB81,
        eRES_0_D=0xCB82,
        eRES_0_E=0xCB83,
        eRES_0_H=0xCB84,
        eRES_0_L=0xCB85,
        eRES_0_HL=0xCB86,
        eRES_0_A=0xCB87,
        eRES_1_B=0xCB88,
        eRES_1_C=0xCB89,
        eRES_1_D=0xCB8A,
        eRES_1_E=0xCB8B,
        eRES_1_H=0xCB8C,
        eRES_1_L=0xCB8D,
        eRES_1_HL=0xCB8E,
        eRES_1_A=0xCB8F,
        eRES_2_B=0xCB90,
        eRES_2_C=0xCB91,
        eRES_2_D=0xCB92,
        eRES_2_E=0xCB93,
        eRES_2_H=0xCB94,
        eRES_2_L=0xCB95,
        eRES_2_HL=0xCB96,
        eRES_2_A=0xCB97,
        eRES_3_B=0xCB98,
        eRES_3_C=0xCB99,
        eRES_3_D=0xCB9A,
        eRES_3_E=0xCB9B,
        eRES_3_H=0xCB9C,
        eRES_3_L=0xCB9D,
        eRES_3_HL=0xCB9E,
        eRES_3_A=0xCB9F,
        eRES_4_B=0xCBA0,
        eRES_4_C=0xCBA1,
        eRES_4_D=0xCBA2,
        eRES_4_E=0xCBA3,
        eRES_4_H=0xCBA4,
        eRES_4_L=0xCBA5,
        eRES_4_HL=0xCBA6,
        eRES_4_A=0xCBA7,
        eRES_5_B=0xCBA8,
        eRES_5_C=0xCBA9,
        eRES_5_D=0xCBAA,
        eRES_5_E=0xCBAB,
        eRES_5_H=0xCBAC,
        eRES_5_L=0xCBAD,
        eRES_5_HL=0xCBAE,
        eRES_5_A=0xCBAF,
        eRES_6_B=0xCBB0,
        eRES_6_C=0xCBB1,
        eRES_6_D=0xCBB2,
        eRES_6_E=0xCBB3,
        eRES_6_H=0xCBB4,
        eRES_6_L=0xCBB5,
        eRES_6_HL=0xCBB6,
        eRES_6_A=0xCBB7,
        eRES_7_B=0xCBB8,
        eRES_7_C=0xCBB9,
        eRES_7_D=0xCBBA,
        eRES_7_E=0xCBBB,
        eRES_7_H=0xCBBC,
        eRES_7_L=0xCBBD,
        eRES_7_HL=0xCBBE,
        eRES_7_A=0xCBBF,
        eSET_0_B=0xCBC0,
        eSET_0_C=0xCBC1,
        eSET_0_D=0xCBC2,
        eSET_0_E=0xCBC3,
        eSET_0_H=0xCBC4,
        eSET_0_L=0xCBC5,
        eSET_0_HL=0xCBC6,
        eSET_0_A=0xCBC7,
        eSET_1_B=0xCBC8,
        eSET_1_C=0xCBC9,
        eSET_1_D=0xCBCA,
        eSET_1_E=0xCBCB,
        eSET_1_H=0xCBCC,
        eSET_1_L=0xCBCD,
        eSET_1_HL=0xCBCE,
        eSET_1_A=0xCBCF,
        eSET_2_B=0xCBD0,
        eSET_2_C=0xCBD1,
        eSET_2_D=0xCBD2,
        eSET_2_E=0xCBD3,
        eSET_2_H=0xCBD4,
        eSET_2_L=0xCBD5,
        eSET_2_HL=0xCBD6,
        eSET_2_A=0xCBD7,
        eSET_3_B=0xCBD8,
        eSET_3_C=0xCBD9,
        eSET_3_D=0xCBDA,
        eSET_3_E=0xCBDB,
        eSET_3_H=0xCBDC,
        eSET_3_L=0xCBDD,
        eSET_3_HL=0xCBDE,
        eSET_3_A=0xCBDF,
        eSET_4_B=0xCBE0,
        eSET_4_C=0xCBE1,
        eSET_4_D=0xCBE2,
        eSET_4_E=0xCBE3,
        eSET_4_H=0xCBE4,
        eSET_4_L=0xCBE5,
        eSET_4_HL=0xCBE6,
        eSET_4_A=0xCBE7,
        eSET_5_B=0xCBE8,
        eSET_5_C=0xCBE9,
        eSET_5_D=0xCBEA,
        eSET_5_E=0xCBEB,
        eSET_5_H=0xCBEC,
        eSET_5_L=0xCBED,
        eSET_5_HL=0xCBEE,
        eSET_5_A=0xCBEF,
        eSET_6_B=0xCBF0,
        eSET_6_C=0xCBF1,
        eSET_6_D=0xCBF2,
        eSET_6_E=0xCBF3,
        eSET_6_H=0xCBF4,
        eSET_6_L=0xCBF5,
        eSET_6_HL=0xCBF6,
        eSET_6_A=0xCBF7,
        eSET_7_B=0xCBF8,
        eSET_7_C=0xCBF9,
        eSET_7_D=0xCBFA,
        eSET_7_E=0xCBFB,
        eSET_7_H=0xCBFC,
        eSET_7_L=0xCBFD,
        eSET_7_HL=0xCBFE,
        eSET_7_A=0xCBFF,
        eCALL_Z_NN=0xCC,
        eCALL_NN=0xCD,
        eADC_A_SHARP=0xCE,
        eRST_08H=0xCF,
        eRET_NC=0xD0,
        ePOP_DE=0xD1,
        eJP_NC_NN=0xD2,

        eCALL_NC_NN=0xD4,
        ePUSH_DE=0xD5,
        eSUB_SHARP=0xD6,
        eRST_10H=0xD7,
        eRET_C=0xD8,
        eRETI=0xD9,
        eJP_C_NN=0xDA,

        eCALL_C_NN=0xDC,
        eSBC_A_SHARP=0xDE,
        eRST_18H=0xDF,
        eLDH_N_A=0xE0,
        ePOP_HL=0xE1,
        eLD_C_A2=0xE2,

        ePUSH_HL=0xE5,
        eAND_SHARP=0xE6,
        eRST_20H=0xE7,
        eADD_SP_SHARP=0xE8,
        eJP_HL=0xE9,
        eLD_NN_A=0xEA,

        eXOR_SHARP=0xEE,
        eRST_28H=0xEF,
        eLDH_A_N=0xF0,
        ePOP_AF=0xF1,
        eLD_A_C2=0xF2,
        eDI=0xF3,

        ePUSH_AF=0xF5,
        eOR_SHARP=0xF6,
        eRST_30H=0xF7,
        eLDHL_SP_N=0xF8,
        eLD_SP_HL=0xF9,
        eLD_A_NN=0xFA,
        eEI=0xFB,

        eCP_SHARP=0xFE,
        eRST_38H=0xFF
      };

    public:
      XCPU();
      ~XCPU();

      /*!
       * @brief Display content of registers.
       * @param opcode OP code executed
       */
      void Display(XMemory* mem, unsigned short opcode);

      void Init();
      void Reset(int gb);

      void Run(XMemory* mem);
      void Test(XMemory* mem);

      /*!
       * @brief Interrupt
       * @author snoogie (1/13/2009)
       * @param int 
       * @return True is handled, false otherwise 
       */
      bool Interrupt(XMemory* mem,int inte);

    private:
      /*!
       * @brief Exec an opcode.
       * @return unsigned char Return cycles need to exec this opcode.
       */
      unsigned char Exec(XMemory* mem);

      /*!
       * @brief Set a flag
       * @author snoogie (1/1/2009)
       * @param flag Flag to set
       */
      void SetFlag(unsigned char flag) { fAF=Hi(fAF)|(Lo(fAF)|flag); }

      /*!
       * @brief Unset a flag
       * @author snoogie (1/1/2009)
       * @param flag Flag to unset
       */
      void UnSetFlag(unsigned char flag) { fAF=Hi(fAF)|(Lo(fAF)&(~flag)); }

      /*!
       * @brief Detect if a flag is set
       * @author snoogie (1/1/2009)
       * @param flag Flag
       * @return true if set, false otherwise
       */
      bool IsSetFlag(unsigned char flag) { return Lo(fAF)&flag; }

      /*!
       * @brief Rotate
       * @author snoogie (1/8/2009)
       * @param reg
       * @param left
       * @param bit
       * @return unsigned char
       */
      unsigned char Rotate(unsigned char reg, bool left, bool bit);

    protected:
      friend class XVideoRam;
      URegister fAF;             //Z N H C 0 0 0 0
      URegister fBC;
      URegister fDE;
      URegister fHL;
      URegister fSP;
      URegister fPC;
      ESpeed fSpeed;
      EStates fState;
      bool fIntFlag;
      bool fEIFlag;
      unsigned char fCycles;
  };
}
#endif                           // __CPU_H__
