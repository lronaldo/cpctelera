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
//    Creates a 256-bytes look-up table for drawing standard screen pixel formatted 
// sprites using a given colour index as transparent.
//
// C Definition:
//    #define <cpctm_createTransparentMaskTable> (*TABLENAME*, *ADDRESS*, *MODE*, *PEN*)
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
//    * This macro may be used several times in different files, resulting in several copies 
// of a same table in memory. There is no way to prevent this, so take care when using this 
// macro several times: be sure of what you want to do.
//    * Any *ADDRESS* value may be used, even addresses that overlap with other parts of your
// own code or data. If this was the case, compiler will complain with "Overlapped record" 
// messages. Take this into account to move your data accordingly, as 2 values cannot share the
// same memory location.
//    * Most of the time, you will require this table to be *memory aligned*. As this table
// takes 256 bytes, it only will be aligned if you place it at any 0x??00 location. If any 
// of the last 2 digits from your 4-digit address is not 0, your table will not be 256-byte aligned.
//
// Size:
//    256 (0x100) bytes
//
// Details:
//    This macro generates a dummy __naked function called dummy_cpct_transparentMaskTable<PEN><MODE>_container
// (without the < > signs, as <PEN> and <MODE> are placeholders for parameters given). The function 
// created contains absolute location assembly code along with the definition of a 256-bytes array (the 
// conversion table) with mask values to be used for generating transparencies out of normal screen
// pixel format values.
//
//    This table will be used by functions like <cpct_drawSpriteMaskedAlignedTable> to make normal sprites
// transparent. The technique is simple: one colour index is considered as transparent. Therefore, pixels
// of this colour form a mask that is used to remove them from the sprite and background. Then, after
// removing transparent pixels, a mixing operation is performed, and the sprite is drawn like if it 
// had an interlaced mask. With this technique, normal sprites may be used as transparent, at the 
// cost of losing one colour. 
//
// Use example:
//    In a mode 0 pirates game, we have three main characters, all of them pirates, that have 
// many animated sprites. These sprites are created in screen pixel format without interlaced
// masks to save a great amount of bytes. To draw these pirates and animations transparent,
// we use the colour 0 (palette index 0) as transparent. For that, we create the transparent
// mask table at a 256-byte aligned memory location (0x2100), and then use that table
// to draw the sprites with the function <cpct_drawSpriteMaskedAlignedTable>:
// (start code)
//    #include <cpctelera.h>
//
//    // Create a 256-byte aligned transparent mask table, for mode 0, 
//    // using palette colour index 0 as transparent
//    cpctm_createTransparentMaskTable(transparentMaskTable, 0x2100, M0, 0);
//
//    // Draws a pirate sprite at a given (X,Y) location as transparent 
//    // All pirates are same size, SPR_W: Sprite Width, SPR_H: Sprite Height
//    drawPirateTransparent(u8* sprite, x, y) {
//       u8* pmem;         // Pointer to video memory location where the pirate will be drawn
//       
//       // Calculate video memory location where to draw the pirate and draw it transparent
//       // Important: Remember that this drawing function requires sprites to be also memory-aligned!
//       pmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
//       cpct_drawSpriteMaskedAlignedTable(sprite, pmem, SPR_W, SPR_H, transparentMaskTable);
//    }
// (end code)
//    This code creates the 256-byte table and includes it in the binary located at address 0x2100 
// (256-bytes aligned, from 0x2100 to 0x21FF, never changing the Most significant byte). Then, the
// function <cpct_drawSpriteMaskedAlignedTable> uses this table to draw pirate sprites as transparent,
// substituting colour 0 in the sprites by the background pixels. 
//
// General recommendations:
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
#define cpctm_createTransparentMaskTable(TABLENAME,ADDRESS,MODE,PEN) \
cpctm_declareMaskTable(TABLENAME); \
void dummy_cpct_transparentMaskTable ## PEN ## MODE ## _container() __naked { \
   __asm \
      .area _ ## TABLENAME ## _ (ABS) \
      .org ADDRESS \
      _ ## TABLENAME:: \
      CPCTM_MASKTABLE ## PEN ## MODE \
      .area _CSEG (REL, CON) \
   __endasm; \
} \
void dummy_cpct_transparentMaskTable ## PEN ## MODE ## _container() __naked

//
// Macro: cpctm_declareMaskTable
//
//    Declares a 256-bytes look-up table for drawing standard screen pixel formatted 
// sprites using a given colour index as transparent. It does not create the table:
// it only declares it to make it accessible from different code files.
//
// C Definition:
//    #define <cpctm_declareMaskTable> (*TABLENAME*)
//
// Parameters:
//    TABLENAME - C-identifier of the table to be declared
//
// Details:
//    This macro generates a declaration for the given *TABLENAME*. This declaration
// is normally expected to be included in a header file so that files including
// the header become able to access the table *TABLENAME*. *TABLENAME* gets
// declared as *extern* and will require to be defined in a source code file.
// If a table is declared using this macro but not defined using 
// <cpctm_createTransparentMaskTable>, a linker error will happen.
//
// Use example:
//    Imagine we have 3 source files and 1 header file: a.c, b.c, t.c and h.h. Both
// a.c and b.c make use of a transparency table named g_transparencyMaskTable, which
// is defined in t.c. For that to be possible, we declare the table in h.h this way:
// (start code)
//    // Include guards
//    #ifndef _H_H_
//    #define _H_H_
//
//    #include <cpctelera.h>
//
//    // Declare g_transparencyMaskTable, which is defined in t.c, and used
//    // in a.c and in b.c also.
//    cpctm_declareMaskTable(g_transparencyMaskTable);
//
//    #endif
// (end code)
//    With this declaration, a.c and b.c only have to include h.h to be able to access
// g_transparencyMaskTable, which is defined in t.c this way:
// (start code)
//    #include "h.h"
//
//    // Create transparency mask table for mode 1 and palette index 1, at address 0x100
//    cpctm_createTransparentMaskTable(g_transparencyMaskTable, 0x100, M1, 1);
// (end code)
//    Then, for instance, a.c. can make use of the table like in this example:
// (start code)
//    #include "h.h"
//
//    //.... code ....
//
//    // Function to draw a transparent sprite
//    drawMyTransparentSprite(u8* sprite, u8* mem_loc) {
//       // All sprites are same width and height
//       cpct_drawSpriteMaskedAlignedTable(sprite, mem_loc, WIDTH, HEIGHT, g_transparencyMaskTable);
//    }
// (end code)
//
#define cpctm_declareMaskTable(TABLENAME) extern const u8 TABLENAME[256]

#endif