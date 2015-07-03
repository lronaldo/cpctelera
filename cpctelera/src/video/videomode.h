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

//
// File: Video Macros&Constants
//

#include <types.h>
#include <memutils/memutils.h>

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

// Using screen coordinates to get byte pointers
extern  u8* cpct_getScreenPtr (void* screen_start, u8 x, u8 y);

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

//
// Macro: cpct_clearScreen
//
//    Macro to simplify clearing the screen.
//
// C Definition:
//   #define <cpct_clearScreen> (*COL*)
//
// Parameters (1 byte):
//   (1B) COL - Colour pattern to be used for screen clearing. Typically, a 0x00 is used 
// to fill up all the screen with 0's (firmware colour 0). However, you may use it in 
// combination with <cpct_px2byteM0>, <cpct_px2byteM1> or a manually created colour pattern.
//
// Details:
//   Fills up all the standard screen (range [0xC000-0xFFFF]) with *COL* byte, the colour 
// pattern given. It uses <cpc_memset> to do the task, just filling up 16K bytes out of
// *COL* value, starting at 0xC000.
//
// Measures:
//    This function takes *98331 microseconds* to fill the screen.
//    This is *4.924 VSYNCs* on a 50Hz display.
//
#define cpct_clearScreen(COL) cpct_memset((void*)0xC000, (COL), 0x4000)

//
// Macro: cpct_clearScreen_f8
//
//    Macro to simplify clearing the screen: fast version (in chuncks of 8 bytes)
//
// C Definition:
//   #define <cpct_clearScreen_f8> (*COL*)
//
// Parameters (2 bytes):
//   (2B) COL - Colour pattern to be used for screen clearing. Typically, a 0x0000 is used 
// to fill up all the screen with 0's (firmware colour 0). However, you may use it in 
// combination with <cpct_px2byteM0>, <cpct_px2byteM1> or a manually created colour pattern.
// Take into account that CPC's memory access is little-endian: this means that using
// 0x1122 as colour pattern will fill up memory with the sequence 0x22, 0x11, 0x22, 0x11...
//
// Details:
//   Fills up all the standard screen (range [0xC000-0xFFFF]) with *COL* pair of bytes, the 
// colour pattern given. It uses <cpc_memset_f8> to do the task, just filling up 16K bytes out 
// of *COL* value, starting at 0xC000.
//
// Warning:
//   <cpc_memset_f8> disables interrupts and moves SP while operating. It also sets interrupts
// to enabled at its end, without taking into account its previous status. Take it into 
// account when using this macro.
//
// Measures:
//    This function takes *41036 microseconds* to fill the screen.
//    This is *2.086 VSYNCs* on a 50Hz display.
//
#define cpct_clearScreen_f8(COL) cpct_memset_f8((void*)0xC000, (COL), 0x4000)

//
// Macro: cpct_clearScreen_f64
//
//    Does exactly the same as <cpct_clearScreen_f8> but calling <cpct_memset_f64> instead
// of <cpct_memset_f8>. Therefore, it works in chuncks of 64 bytes, being a 33% faster. 
//
// C Definition:
//   #define <cpct_clearScreen_f64> (*COL*)
//
// Parameters (2 bytes):
//   (2B) COL - Colour pattern to be used for screen clearing. Typically, a 0x0000 is used 
// to fill up all the screen with 0's (firmware colour 0). However, you may use it in 
// combination with <cpct_px2byteM0>, <cpct_px2byteM1> or a manually created colour pattern.
// Take into account that CPC's memory access is little-endian: this means that using
// 0x1122 as colour pattern will fill up memory with the sequence 0x22, 0x11, 0x22, 0x11...
//
// Details:
//   Fills up all the standard screen (range [0xC000-0xFFFF]) with *COL* pair of bytes, the 
// colour pattern given. It uses <cpc_memset_f64> to do the task, just filling up 16K bytes out 
// of *COL* value, starting at 0xC000.
//
// Warning:
//   <cpc_memset_f64> disables interrupts and moves SP while operating. It also sets interrupts
// to enabled at its end, without taking into account its previous status. Take it into 
// account when using this macro.
//
// Measures:
//    This function takes *33843 microseconds* to fill the screen.
//    This is *1.721 VSYNCs* on a 50Hz display.
//
#define cpct_clearScreen_f64(COL) cpct_memset_f64((void*)0xC000, (COL), 0x4000)

#endif
