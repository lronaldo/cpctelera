//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//#####################################################################
//### MODULE: Relocation Utilities                                  ###
//#####################################################################
//### Utilities to locate or relocate things into memory, or to     ###
//### establish memory location where to load sections of a binary. ###
//#####################################################################
//

#ifndef CPCT_RELOCATIONUTILS_H
#define CPCT_RELOCATIONUTILS_H

#include <types.h>

// Useful macros to concatenate names of identifiers
#define CONCAT3(A, B, C)     A ## B ## C
#define NAMECONCAT3(A, B, C) CONCAT3(A, B, C)

//
// Macro: CPCT_ABSOLUTE_LOCATION_AREA(MEM)
//
//    Macro that produces next code to be located at absolute memory
// location MEM. 
//
// C Definition:
//    #define <CPCT_ABSOLUTE_LOCATION_AREA> (*MEM*)
//
// Input Parameters (2 bytes):
//    (2B) MEM - Memory location where code produced next will be placed
//
// Parameter Restrictions:
//    * *MEM* could be any 16-bits value from 0x0000 to 0xFFFF. Passing
// A value lower than 0 or greater than 0xFFFF may produced unpredictable
// behaviour.
//
// Warnings:
//    * *MEM* should be different to any other *MEM* value used on previous
// calls to this macro.
//    * It is up to the programmer to locate code on available memory places.
// If *MEM* is not carefully selected, it could be placed overlapping other
// code segments. On this event, part of the code will be shadowed (it will
// never be loaded in memory) as it is impossible to load 2 values on the
// same memory cell.
//    * This function should be used OUTSIDE the scope of any function.
//
// Details:
//    This macro is used to change the location where subsequent code or
// data will be located when the binary gets loaded. All code or data
// written after the call to this macro will be loaded from *MEM* on.
//
//    An example of use of this macro would be as follows:
// (start code)
//    //
//    // .. previous code, dynamically placed in memory by the compiler
//    //
//
//    CPCT_ABSOLUTE_LOCATION_AREA(0x0040);
//
//    // 1000 bytes of music to be located staring at 0x0040
//    u8 music_data[1000] = { 0x0A, 0x12, 0x5F, 
//    // .....
//    }
//    
//    CPCT_ABSOLUTE_LOCATION_AREA(0x8000);
//
//    // Function code will be placed at 0x8000
//    void game_loop(u8 parameter) {
//       // function code...   
//    }
// (end code)
//    Several absolutelly located code areas may be created, but they 
// should be located at different memory locations.
//
//    The function creates dummy functions and locates assembly code
// for relocation inside these dummy functions. These functions are
// not to be called by any means, nor it is required; they are required
// as the compiler prevents directives from being entered outside function
// scope. These functions are named as "dummy_absolute_MEM" and they
// cannot be duplicated, as they are proper functions for the compiler. 
//
#define CPCT_ABSOLUTE_LOCATION_AREA(MEM) \
void dummy_absolute_ ## MEM (void) __naked { \
  __asm \
    .area _ ## MEM ## _ (ABS, CON) \
    .org MEM \
  __endasm; \
} \
void dummy_absolute_##MEM (void) __naked

#undef _CODE 
#define CPCT_RELATIVE_LOCATION_AREA(FNAME) \
void NAMECONCAT3(dummy_relocatable_, __LINE__, FNAME) (void) __naked { \
  __asm \
    .area _CODE \
  __endasm; \
} \
void NAMECONCAT3(dummy_relocatable_, __LINE__, FNAME) (void) __naked

#endif
