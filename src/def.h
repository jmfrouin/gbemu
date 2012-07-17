/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#ifndef __DEF_H__
#define __DEF_H__

#define NAME "gbemu"
#define VERSION 0.59

#define GB_CARTRIDGE_SIZE 32768
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144
#define GB_SCREEN_DEPTH 16

#define GB_BLACK_PIXEL 0
#define GB_WHITE_PIXEL ~0

#define ROM_START 0x0000
#define ROM_END 0x7FFF
#define VIDEO_RAM_START 0x8000
#define VIDEO_RAM_END 0x9FFF
#define SWITCHABLE_RAM_START 0xA000
#define SWITCHABLE_RAM_END 0xBFFF
#define RAM_START 0xC000
#define RAM_DUPLICATE 0xE000
#define RAM_END 0xFDFF

#include "errors.h"
#endif                           //__DEF_H__
