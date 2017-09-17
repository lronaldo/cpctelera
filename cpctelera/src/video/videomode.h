//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------

#ifndef CPCT_VIDEOMODE_H
#define CPCT_VIDEOMODE_H

#include <types.h>
#include <memutils/memutils.h>
#include "colours.h"
#include "video_macros.h"

// Setting Video Mode
extern void cpct_setVideoMode (u8 videoMode) __z88dk_fastcall;

// Waiting for VSYNC
extern void cpct_waitVSYNC    ();
extern  u16 cpct_count2VSYNC  ();

// Palette functions
extern void cpct_fw2hw        (void *fw_colour_array, u16 size) __z88dk_callee;
extern void cpct_setPalette   (u8* ink_array, u16 ink_array_size) __z88dk_callee;
extern   u8 cpct_getHWColour  (u16 firmware_colour) __z88dk_fastcall;
extern void cpct_setPALColour (u8 pen, u8 hw_ink) __z88dk_callee;

// Functions to modify video memory location
extern void cpct_setVideoMemoryPage   (u8 page_codified_in_6LSb) __z88dk_fastcall;
extern void cpct_setVideoMemoryOffset (u8 offset) __z88dk_fastcall;

// Using screen coordinates to get byte pointers
extern  u8* cpct_getScreenPtr (void* screen_start, u8 x, u8 y) __z88dk_callee;

#endif
