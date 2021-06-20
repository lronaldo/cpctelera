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

#ifndef _CPCT_VIDEO_MACROS_H
#define _CPCT_VIDEO_MACROS_H

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// File: Macros (C)
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Group: Video memory manipulation
////////////////////////////////////////////////////////////////////////

//
// Constant: CPCT_VMEM_START
//
//    The address where screen video memory starts by default in the Amstrad CPC.
//
//    This address is exactly 0xC000, and this macro represents this number but
// automatically converted to <u8>* (Pointer to unsigned byte). You can use this
// macro for any function requiring the start of video memory, like 
// <cpct_getScreenPtr>.
//
#define CPCT_VMEM_START (u8*)0xC000

//
// Constants: Video Memory Pages
//
// Useful constants defining some typical Video Memory Pages to be used as 
// parameters for <cpct_setVideoMemoryPage>
//
// cpct_pageCO - Video Memory Page 0xC0 (0xC0··)
// cpct_page8O - Video Memory Page 0x80 (0x80··)
// cpct_page4O - Video Memory Page 0x40 (0x40··)
// cpct_page0O - Video Memory Page 0x00 (0x00··)
//
#define cpct_pageC0 0x30
#define cpct_page80 0x20
#define cpct_page40 0x10
#define cpct_page00 0x00

//
// Macro: cpctm_memPage6
//
//    Macro that encodes a video memory page in the 6 Least Significant bits (LSb)
// of a byte, required as parameter for <cpct_setVideoMemoryPage>
//
// C Definition:
// #define <cpctm_memPage6> (*PAGE*)
//
// Parameters (1 byte):
// (1B) PAGE - Video memory page wanted 
//
// Returns:
//  u8   - Video Memory Page encoded in the 6 LSb of the byte.
//
// Details:
//  This is just a macro that shifts *PAGE* 2 bits to the right, to leave it
// with just 6 significant bits. For more information, check functions
// <cpct_setVideoMemoryPage> and <cpct_setVideoMemoryOffset>.
//
#define cpctm_memPage6(PAGE) ((PAGE) >> 2)

// Deprecated, but maintained for compatibility
#define cpct_memPage6(PAGE) ((PAGE) >> 2)

//
// Macro: cpctm_screenPtr
//
//    Macro that calculates the video memory location (byte pointer) of a 
// given pair of coordinates (*X*, *Y*)
//
// C Definition:
//    #define <cpctm_screenPtr> (*VMEM*, *X*, *Y*)
//
// Parameters:
//    (2B) VMEM - Start of video memory buffer where (*X*, *Y*) coordinates will be calculated
//    (1B) X    - X Coordinate of the video memory location *in bytes* (*BEWARE! NOT in pixels!*)
//    (1B) Y    - Y Coordinate of the video memory location in pixels / bytes (they are same amount)
//
// Parameter Restrictions:
//    * *VMEM* will normally be the start of the video memory buffer where you want to 
// draw something. It could theoretically be any 16-bits value. 
//    * *X* must be in the range [0-79] for normal screen sizes (modes 0,1,2). Screen is
// always 80 bytes wide in these modes and this function is byte-aligned, so you have to 
// give it a byte coordinate (*NOT a pixel one!*).
//    * *Y* must be in the range [0-199] for normal screen sizes (modes 0,1,2). Screen is 
// always 200 pixels high in these modes. Pixels and bytes always coincide in vertical
// resolution, so this coordinate is the same in bytes that in pixels.
//    * If you give incorrect values to this function, the returned pointer could
// point anywhere in memory. This function will not cause any damage by itself, 
// but you may destroy important parts of your memory if you use its result to 
// write to memory, and you gave incorrect parameters by mistake. Take always
// care.
//
// Returns:
//    void * - Pointer to the (*X*, *Y*) location in the video buffer that starts at *VMEM*
//
// Details:
//    This macro does the same calculation than the function <cpct_getScreenPtr>. However,
// as it is a macro, if all 3 parameters (*VMEM*, *X*, *Y*) are constants, the calculation
// will be done at compile-time. This will free the binary from code or data, just puting in
// the result of this calculation (2 bytes with the resulting address). It is highly 
// recommended to use this macro instead of the function <cpct_getScreenPtr> when values
// involved are all constant. 
//
//    Take care of using this macro with variable values. In this latest case, the compiler 
// will generate in-place code for doing the calculation. Therefore, that will take binary
// space for the code and CPU time for the calculation. Moreover, calculation will be slower
// than if it were done using <cpct_getScreenPtr> and code could be duplicated if this macro
// is used in several places. Therefore, for variable values, <cpct_getScreenPtr> is recommended.
//
//    Sum up of recommendations:
//    All constant values - Use this macro <cpctm_screenPtr>
//    Any variable value  - Use the function <cpct_getScreenPtr>
//
#define cpctm_screenPtr(VMEM,X,Y) (void*)((VMEM) + 80 * ((unsigned int)((Y) >> 3)) + 2048 * ((Y) & 7) + (X))

////////////////////////////////////////////////////////////////////////
// Group: Setting the border
////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////
// Group: Clearing screen
////////////////////////////////////////////////////////////////////////

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
