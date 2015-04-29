//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------

#ifndef CPCT_VIDEOMODE_H
#define CPCT_VIDEOMODE_H

#include <types.h>

// Setting Video Mode
extern void cpct_setVideoMode (u8 videoMode);

// Waiting for VSYNC
extern void cpct_waitVSYNC    ();
extern  u16 cpct_count2VSYNC  ();

// Palette functions
extern void cpct_fw2hw        (void *fw_colour_array, u8 size);
extern void cpct_setPalette   (u8* ink_array, u8 ink_array_size);
extern   u8 cpct_getHWColour  (u8 firmware_colour);
extern void cpct_setPALColour (u8 pen, u8 hw_ink);

// Functions to modify video memory location
extern void cpct_setVideoMemoryPage   (u8 page_codified_in_6LSb);
extern void cpct_setVideoMemoryOffset (u8 offset);

///
/// Macro: cpct_setBorder
///
///   Changes the colour of the screen border.
///
/// C Definition:
///   #define <cpct_setBorder> (HWC)  <cpct_setPALColour> (16, (HWC))
///
/// Input Parameters (1 Byte):
///   (1B) HWC - Hardware colour value for the screen border.
///
/// More information:
///   This is not a real function, but a C macro. Beware of using it along
/// with complex expressions or calculations, as it may expand in non-desired
/// ways.
///
///   For more information, check the real function <cpct_setPALColour>, which
/// is called when using *cpct_setBorderColour* (It is called using 16 as *pen*
/// argument, which identifies the border).
///
#define cpct_setBorder(HW_C) cpct_setPALColour(16, (HW_C))


#endif
