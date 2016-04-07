//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#ifndef _TRANSPARENCY_TABLE_MACROS_H
#define _TRANSPARENCY_TABLE_MACROS_H

//----------------------------------------------------------------------------------------
// Title: Transparency Macros
//----------------------------------------------------------------------------------------

#include <types.h>
#include "transparency_tables_m0.h"
#include "transparency_tables_m1.h"

//
// Macro: cpctm_createTransparentMaskTable
//
//    Creates a 256-bytes look-up table for drawing standard mode 0 screen pixel formatted 
// sprites using colour index 0 as transparent.
//
// C Definition:
//    #define <cpctm_createTransparentMaskTable>(*TABLENAME*, *ADDRESS*, *MODE*, *PEN*)
//
// Parameters:
//    TABLENAME - C-identifier to be used as name for this table
//    ADDRESS   - Memory address where the start of the table will be located. Take special 
// care with this value not to overlap other parts of the code.
//    MODE      - It must be either *M0* or *M1* (M capital)
//    PEN       - It must be a *decimal* value from 0 to 15 (from 0 to 3 for mode 1) *without*
// *trailing zeros*.
// 
// Known limitations:
//    * Macro may be used several times, resulting in several copies of the same table in 
// memory. There is no way to prevent this, so take care when using this macro several times:
// be sure of what you want to do.
//    * Any *ADDRESS* value may be used, even addresses that overlap with other parts of your
// own code or data. If this was the case, compiler will complain with "Overlapped record" 
// messages. Take this into account to move your data accordingly, as 2 values cannot share the
// same memory location.
//    * Most of the time, you will require this table to be *memory aligned*. As this table
// takes 256 bytes, it only will be aligned if you place it at any 0x??00 location. If any 
// of the last 2 digits from your 4-digit address is not 0, your table will not be aligned.
//
// Details:
//    This macro generates a dummy __naked function called dummy_cpct_transparentMaskTable00M0_container
// containing absolute location assembly code along with the definition of a 256 array with 
// mask values to be used 
// (start code)
//    // Include transparency table for mode 0 at 0x2100 (0x2100 - 0x21FF)
//    #define cpct_transparentMaskTable00M0_address 0x2100
//    #include <sprites/transparency_tables.h>
// (end code)
//    With this definition, you are including <cpct_transparentMaskTable00M0> in your binary
// at address 0x2100 (256-bytes aligned, from 0x2100 to 0x21FF, never changing the Most significant byte). 
// You are always required to provide a concrete address for this table at definition time. 
//
// General recommendations:
//    * Be vigilant with the use of explicit addresses in SDCC. It is recommended that you group 
// together all explicitly located items in one single file. Otherwise, memory locations may be 
// messed up in the final binary by the linker.
//    * Remember that locating items at a specific memory location is done writing them at a concrete
// location in the final binary, taking into account where it will be loaded. That can make the size
// of the final binary increase, and even overwrite parts of the memory you did not intended. For instance,
// imagine you have a binary with 1024 (0x400) bytes of code that you load at memory address 0x1000.
// That binary occupies memory from 0x1000 to 0x1400 when loaded. If you try to explicitly place 
// the transparent table at location 0x8000 in memory, what the compiler does is creating a 28K 
// binary, with code at first 1024 (0x400) bytes, 27K of zeros (from 0x1400 to 0x8000) and then 
// 256 (0x100) bytes with the table. That could erase unintended things in memory when loading.
//    * Always do your own calculations to prevent explicitly placing things overlapped. It is
// recommended that you put your explicitly located data items first, previous to the starting
// memory address of your program. That's easier to manage.
//
// Size:
//    256 (0x100) bytes
//
// Details:
//    Look-up table (array) containing all the masks for the 256 possible combinations of
// byte pixel data values, assuming firmware colour 0 as being transparent. It is used by
// sprite-drawing functions to draw the sprites using colour 0 as transparent.
// 
//    This table should be placed at a specific location in memory. It is recommended to place 
// it at a 256-byte aligned memory location (a memory location whose Least signficant byte is 0, 
// so the address has the form 0x??00). In fact, some functions like <cpct_drawSpriteMaskedAlignedTable> 
// require it to be 256-byte aligned or they will fail.
// 
//    To be able to use this table, all you have to do is defining the macro (constant value) 
// <cpct_transparentMaskTable00M0_address> before the inclusion of *<sprites/transparency_tables.h>*, 
// CPCtelera's header file.
//
//    Once the table has been defined, you can access it through the symbol <cpct_transparentMaskTable00M0>.
//
//
//
// Macro: cpct_transparentMaskTable00M0_address
//
//    Macro used to set the location where <cpct_transparentMaskTable00M0> must be included in memory.
// When this macro is defined, <cpct_transparentMaskTable00M0> is included.
//
#define cpctm_defineMaskTable(TABLE) extern const u8 TABLE[256]

#define cpctm_createTransparentMaskTable(TABLE,ADDRESS,MODE,PEN) \
cpctm_defineMaskTable(TABLE); \
void dummy_cpct_transparentMaskTable ## PEN ## MODE ## _container() __naked { \
   __asm \
      .area _ ## TABLE ## _ (ABS) \
      .org ADDRESS \
      _ ## TABLE:: \
      CPCTM_MASKTABLE ## PEN ## MODE \
      .area _CSEG (REL, CON) \
   __endasm; \
} \
void dummy_cpct_transparentMaskTable ## PEN ## MODE ## _container() __naked

#endif