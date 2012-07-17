/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include "timer.h"

namespace Hardware
{
  XTimer::XTimer():
  fFreq(eLow), fCycles(0)
  {
    Reset();
  }

  XTimer::~XTimer()
  {
  }

  void XTimer::Init()
  {
    Reset();
  }

  void XTimer::Reset()
  {
    fFreq=eLow;
    fCycles=0;
  }

  void XTimer::Update()
  {
  }
}
