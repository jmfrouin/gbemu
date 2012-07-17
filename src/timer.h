/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __TIMER_H__
#define __TIMER_H__

namespace Hardware
{
  /*!
   * @brief GB's Timer.
   */
  class XTimer
  {
    public:
      enum EFrequencies
      {
        eLow=4096,
        eNormal=16384,
        eHigh=65536,
        eVeryHigh=242144
      };
      XTimer();
      ~XTimer();

      void Init();
      void Reset();
      void Update();
      unsigned int Cycles() { return fCycles; }

    private:
      EFrequencies fFreq;
      unsigned int fCycles;
  };
}
#endif                           //__TIMER_H__
