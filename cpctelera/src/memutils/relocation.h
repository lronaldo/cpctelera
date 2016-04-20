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


//
// Title: Memory Relocation Utilities
//
// Utilities to locate or relocate things into memory, or to 
// establish memory location where to load different areas of a binary. 
//

#ifndef CPCT_RELOCATIONUTILS_H
#define CPCT_RELOCATIONUTILS_H

#include <types.h>

// Useful macros to concatenate names of identifiers
#define CONCAT3(A, B, C)     A ## B ## C
#define NAMECONCAT3(A, B, C) CONCAT3(A, B, C)

//
// Macro: CPCT_ABSOLUTE_LOCATION_AREA
//
//    Macro that produces following code and data to be located at 
// given absolute memory location MEM. 
//
// C Definition:
//    #define <CPCT_ABSOLUTE_LOCATION_AREA> (*MEM*)
//
// Input Parameters (2 bytes):
//    (2B) MEM - Memory location where code produced next will be placed
//
// Parameter Restrictions:
//    * *MEM* could be any 16-bits value from 0x0000 to 0xFFFF. Passing
// any value lower than 0 or greater than 0xFFFF will produce unpredictable
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
//    * This macro should be used *outside* the scope of any function.
//    * This macro *should not be used to* set the code entry point (i.e. the
// place where compiler starts generating the code). For this purpose you 
// should used *Z80CODELOC* variable instead (located in cfg/build_config.mk).
//    * Beware when using this macro on files not containing code (only data, 
// like arrays, strings, etc.). In this case, if you use only once and previous
// to all data definitions, it would probably fail. This is due to the compiler
// adding an ".area _CODE" directive to files that do not contain functions.
// To overcome this problem you should add another macro at the end or 
// in the middle of the file. You can use <CPCT_ABSOLUTE_LOCATION_AREA> again
// or <CPCT_RELOCATABLE_AREA>, to prevent the compiler from rearranging your data.
// This is an example on how to do it:
// (start code)
//    // This is the start of a file called music.c
//    CPCT_ABSOLUTE_LOCATION_AREA(0x040);
//
//    // Music data gets located at 0x040
//    const u8 music_data[100] = { 0x41, 0x54, 0x31, .... };
//
//    // Failing to add a relocation macro here will produce 
//    // the compiler to add a ".area _CODE" directive before 
//    // music data. This happens in data-only files.
//    CPCT_RELOCATABLE_AREA();
//
//    // This is the end of the file music.c
// (end code)
// 
// Required memory:
//    1 byte
//
// Details:
//    This macro is used to change the location where subsequent code or
// data will be located when the binary gets loaded. All code or data
// written *after* the call to this macro will be loaded from *MEM* on.
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
//    const u8 music_data[1000] = { 0x0A, 0x12, 0x5F, 
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
//    Several absolutely located code areas may be created, but they 
// should be placed at different memory locations.
//
//    This macro creates dummy functions and produces assembly code
// for relocation inside these dummy functions. These functions are
// not to be called by any means, nor it is required; they are required
// as the compiler prevents directives from being entered outside function
// scope. These functions are named as /dummy_absolute_MEM/ and 
// /dummy_data_absorber_MEM/ and they cannot be duplicated, as they are 
// proper functions for the compiler. 
//
//    /dummy_data_absorber_MEM/ function is generated first, and its 
// purpose is to make all previous data definitions (arrays, strings, etc)
// to be "absorbed". With this function being defined, previous data 
// gets placed by the compiler at the end of this "absorber" function, 
// just before the new absolutely located area definition, as wanted.
// This function only contains a RET statement, and takes up 1 byte of
// space in the final binary. As this function is dummy and gets never
// called, this additional byte can be safely removed or overlapped if
// required. This may by done by placing next absolutely located area
// exactly where this byte lies, or by editing produced assembly code
// and removing the RET statement before compiling. Do these operations
// only if you know exactly what you are doing.
//
#define CPCT_ABSOLUTE_LOCATION_AREA(MEM) \
void dummy_data_absorber##MEM (void) {} \
void dummy_absolute_##MEM (void) __naked { \
  __asm \
    .area _ ## MEM ## _ (ABS) \
    .org MEM \
  __endasm; \
} \
void dummy_absolute_##MEM (void) __naked

//
// Macro: CPCT_RELOCATABLE_AREA
//
//    Macro that produces following code to be automatically distributed
// by the linker amongst available memory space area, starting in the
// loading location defined by Z80CODELOC (see cfg/build_config.mk file)
//
// C Definition:
//    #define <CPCT_RELOCATABLE_AREA> (*ID*)
//
// Input Parameters (identifier):
//    (identifier) ID - An *optional* identifier to distinguish container
// functions for relative location areas.
//
// Parameter Restrictions:
//    * *ID* could be any valid identifier of, at most, 16 characters length.
// This identifier is optional, and only required when name collisions do
// appear.
//
// Warnings:
//    * *ID* parameter is not mandatory, but optional.
//    * This macro should be used *outside* the scope of any function.
//    * Read section *Details* if you have compilation problems using this
// macro. There are possible issues when using the macro across different
// source files.
//
// Required memory:
//    1 byte
//
// Details:
//    This macro restores normal code location behaviour. This normal behaviour
// sets the linker to decide where should code and data be placed, inside
// the relative code area. Relative code area starts at the binary loading
// point (Z80CODELOC) and extends from there to the end of memory. Starting
// place for this code area can be changed at compile-time by editing
// the file cfg/build_config.mk of your CPCtelera project, and assigning
// desired memory location to Z80CODELOC variable.
//
//    Every time this macro is called, following code will be added to the
// general _CODE area. This area is the global relocatable area. This means
// that all the code contained in this area can be relocated as linker considers.
//
//    An example of use of this macro would be as follows:
// (start code)
//    //
//    // First part of a C file. All code is added to _CODE area by
//    // default, and located by the linker from Z80CODELOC onwards.
//    // So, following code will be relocated by the linker as required.
//    //
//    void drawCompound(u8* sprite) {
//       // .. Code for drawing a compound. Compiled code
//       // .. will be placed by the linker in the _CODE area, 
//       // .. from Z80CODELOC onwards
//    }
//
//    CPCT_ABSOLUTE_LOCATION_AREA(0x0040);
//
//    //
//    // .. This array of data will be placed at 0x0040 absolute location onwards
//    // 
//    const u8 music_data[1000] = { 0x0A, 0x12, 0x5F, 
//       // .. 1000 bytes of data
//    };
//    
//    CPCT_RELOCATABLE_AREA();
//
//    // 
//    // Next data and functions will be added to the _CODE area, 
//    // same as the drawCompound function. All will be placed by 
//    // the linker as it considers, inside the relocatable area 
//    // that starts at Z80CODELOC
//    //
//    const u8 character_info[5] = { 10, 52, 100, -1, 3 };
//    void game_loop(u8 parameter) {
//       // function code...
//    }
//
//    CPCT_ABSOLUTE_LOCATION_AREA(0x1040);
//
//    //
//    // Next code will be placed from 0x1040 onwards, just after
//    // music data (ensuring that it is not overlapped)
//    // 
//    void playMusic() {
//       // function code...
//    }
//
//    CPCT_RELOCATABLE_AREA();
//
//    // Main function will be placed in relative _CODE area managed
//    // by the linker. 
//    void main() {
//       // main code
//    }
//
// (end code)
//
//    This macro may be used as many times as required. An indefinite
// number of code and data areas can be flagged as relative to be
// managed by the linker. 
//
//    This macro creates dummy functions and produces assembly code
// for relocation inside these dummy functions. These functions are
// not to be called by any means, nor it is required; they are required
// as the compiler prevents directives from being entered outside function
// scope. These functions are named as /dummy_relative___LINE__ID/ and 
// /dummy_data_absorber___LINE__ID/ and they cannot be duplicated, 
// as they are proper functions for the compiler. 
//
//    /dummy_data_absorber___LINE__ID/ function is generated first, and its 
// purpose is to make all previous data definitions (arrays, strings, etc)
// to be "absorbed". With this function being defined, previous data 
// gets placed by the compiler at the end of this "absorber" function, 
// just before the new absolutely located area definition, as wanted.
// This function only contains a RET statement, and takes up 1 byte of
// space in the final binary. As this function is dummy and gets never
// called, this additional byte can be safely removed or overlapped if
// required. This may by done by placing next absolutely located area
// exactly where this byte lies, or by editing produced assembly code
// and removing the RET statement before compiling. Do these operations
// only if you know exactly what you are doing.
//
//    When using this macro on different source code files, a compilation
// error may arise. On the event of having 2 uses of this macro, on
// the same source code line number, but in different files, they will
// produce the same function name, unless different IDs are provided. 
// In this case, a compilation error will happen. There are 2 possible
// solutions,
//    -  Add source code lines to place macros at different source code lines
//    -  Add 2 different IDs on the call to <CPCT_RELOCATABLE_AREA> (ID).
// This will prevent names from clashing.
//
#define CPCT_RELOCATABLE_AREA(FNAME) \
void NAMECONCAT3(dummy_data_absorber_, __LINE__, FNAME) (void) {} \
void NAMECONCAT3(dummy_relocatable_, __LINE__, FNAME) (void) __naked { \
  __asm \
    .area _CSEG (REL, CON) \
  __endasm; \
} \
void NAMECONCAT3(dummy_relocatable_, __LINE__, FNAME) (void) __naked

#endif
