/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __ERRORS_H__
#define __ERRORS_H__

namespace Hardware
{
  enum EErrors
  {
    eNoError,
    eCannotSave,
    eCannotLoad,
    #if defined WITH_X11
    eSurfaceCannotOpenDisplay,
    eSurfaceCannotCreateWindow,
    eSurfaceCannotGetFont,
    #endif
    #if defined WITH_SDL
    eSDLInitVideoFailed,
    eSDLCannotSetVideoMode,
    eSDLCannotCreateRGBSurface,
    #endif
  };
}
#endif                           //__ERRORS_H__
