//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//#####################################################################
//### MODULE: Sprites
//### SUBMODULE: flipping.table_macros
//#####################################################################
//### Macros used to generate pixel and sprite flipping tables
//#####################################################################
//

#ifndef CPCT_SPRITEFLIPPING_TABLE_MACROS_H
#define CPCT_SPRITEFLIPPING_TABLE_MACROS_H

#include <types.h>
#include "flipping_tables.h"

//----------------------------------------------------------------------------------------
// Title: Pixel Horizontally Flipping Macros
//----------------------------------------------------------------------------------------

//
// Macro: cpctm_createPixelFlippingTable
//
//    Creates a 256-bytes look-up table to be used in horizontally flipping pixels.
// Horizontally flipping functions make use of this table.
//
// C Definition:
//    #define <cpctm_createPixelFlippingTable> (*TABLENAME*, *ADDRESS*, *MODE*)
//
// Parameters:
//    TABLENAME - C-identifier to be used as name for this table
//    ADDRESS   - Memory address where the start of the table will be located. Take special 
// care with this value not to overlap other parts of the code.
//    MODE      - Must be a value from this set { *M0*, *M1*, *M2* } (M capital)
// 
// Known limitations:
//    * This macro may be used several times in different files, resulting in several copies 
// of a same table in memory. There is no way to prevent this, so take care when using this 
// macro several times: be sure of what you want to do.
//    * Any *ADDRESS* value may be used, including addresses that overlap with other parts of your
// own code or data. If this was the case, compiler will complain with "Overlapped record" 
// messages. Take this into account to move your data accordingly, as 2 values cannot share the
// same memory location.
//    * Most of the time, you will require this table to be *memory aligned*. As this table
// takes 256 bytes, it only will be aligned if you place it at any 0x??00 location. If any 
// of the last 2 digits from your 4-digit hexadecimal address is not 0, your table 
// will not be 256-byte aligned. That will make functions perform erratically, as they 
// expect the table to be 256-byte aligned.
//
// Size:
//    256 (0x100) bytes
//
// Details:
//    This macro generates a dummy __naked function called cpctm_createPixelFlippingTable<MODE>_container
// (without the < > signs, <MODE> is a placeholder for parameter given). The function 
// created contains absolute location assembly code along with the definition of a 256-bytes array 
// (the table) with all pixel flipping values for each possible byte encoded in screen pixel format.
//
//    This table will be used by functions like <cpct_drawSpriteHFlipM0_at> to draw normal mode 0 sprites
// horizontally flipped. The technique is simple: each byte encodes 2-to-8 pixels (depending on 
// screen mode) left-to-right. The table is a precalculation of all possible combinations of these
// pixels flipped. Therefore, for each byte of pixels you wanted to flip, you use the byte as index in
// the table to retrieve the precalculated flipped version of your byte.
//
// Use example:
//    In a mode 1 space game, we have some spaceships that move left-to-right and right-to-left. 
// Game data only includes sprites for spaceships looking to the right. In order to draw 
// spaceships looking to the left, a function like <cpct_drawSpriteHFlipM1_at> has to be used.
// For that, we create the pixel flipping table at a 256-byte aligned memory location (0x7F00), 
// and then use that table to draw the sprites with <cpct_drawSpriteHFlipM1_at>:
// (start code)
//    #include <cpctelera.h>
//
//    // Definition of the Entity struct
//    typedef struct Ent {
//       u8*   sprite;        // Sprite of the entity
//       u8    w, h;          // Width and Height of the sprite (in bytes)
//       u8    x, y;          // Coordinates of the entity
//       u8    lookingRight;  // True if Entity is looking Right
//    } Entity;
//
//    // Create a 256-byte aligned pixel flipping table, for mode 1, 
//    cpctm_createPixelFlippingTable(g_flipTable, 0x7F00, M1);
//
//    // Draws a spaceship sprite at a given (X,Y) location taking into account
//    // if the spaceship looks right or left. 
//    drawSpaceship(Entity* e) {
//       u8* pmem;         // Pointer to video memory location where the pirate will be drawn
//       
//       // Calculate video memory location where to draw the spaceship
//       pmem = cpct_getScreenPtr(CPCT_VMEM_START, e->x, e->y);
//
//       // Draw the spaceship depending to where it is looking at
//       if (e->lookingRight) {
//          // It is looking Right, draw it normal
//          cpct_drawSprite(e->sprite, pmem, e->w, e->h);
//       } else {
//          // It is looking left, draw it horizontally flipped
//          cpct_drawSpriteHFlip_at(e->sprite, pmem, e->w, e->h, g_flipTable);
//       }
//    }
// (end code)
//    This code creates the 256-byte table and includes it in the binary located at address 0x7F00
// (256-bytes aligned, from 0x7F00 to 0x7FFF. The Most Significant byte never changes). Then, the
// function <cpct_drawSpriteHFlipM1_at> uses this table to draw . 
//
// General recommendations:
//    * Remember that locating items at a specific memory location is done writing them at a concrete
// location in the final binary, taking into account where it will be loaded. That can make the size
// of the final binary increase, and even overwrite parts of the memory you did not intended to. 
// For instance, imagine you have a binary with 1024 (0x400) bytes of code that you load at memory 
// address 0x1000. That binary occupies memory from 0x1000 to 0x1400 when loaded. If you try to 
// explicitly place the transparent table at location 0x8000 in memory, what the compiler does
// is creating a 28K binary, with code at first 1024 (0x400) bytes, 27K of zeros (from 0x1400 to 
// 0x8000) and then 256 (0x100) bytes with the table. That could erase unintended things in memory 
// when loading.
//    * Always do your own calculations to prevent explicitly placing things overlapped. It is
// recommended that you put your explicitly located data items first, previous to the starting
// memory address of your program. That's easier to manage.
//
#define cpctm_createPixelFlippingTable(TABLENAME,ADDRESS,MODE) \
cpctm_declarePixelFlippingTable(TABLENAME); \
void dummy_cpct_PixelFlippingTable ## MODE ## _container() __naked { \
   __asm \
      .area _ ## TABLENAME ## _ (ABS) \
      .org ADDRESS \
      _ ## TABLENAME:: \
      CPCT_PIXEL_FLIPPING_TABLE_ ## MODE \
      .area _CSEG (REL, CON) \
   __endasm; \
} \
void dummy_cpct_PixelFlippingTable ## MODE ## _container() __naked

//
// Macro: cpctm_declarePixelFlippingTable
//
//    Declares a 256-bytes look-up table for horizontally flipping pixels in a byte. 
// It does not create the table: it only declares it to make it accessible from 
// different code files.
//
// C Definition:
//    #define <cpctm_declarePixelFlippingTable> (*TABLENAME*)
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
// <cpctm_createPixelFlippingTable>, a linker error will happen.
//
// Use example:
//    Imagine we have 3 source files and 1 header file: a.c, b.c, t.c and h.h. Both
// a.c and b.c make use of a horizontally flipping table named gflipTable, which
// is defined in t.c. For that to be possible, we declare the table in h.h this way:
// (start code)
//    // Include guards
//    #ifndef _H_H_
//    #define _H_H_
//
//    #include <cpctelera.h>
//
//    // Declare gflipTable, which is defined in t.c, and used
//    // in a.c and in b.c also.
//    cpctm_declarePixelFlippingTable(gflipTable);
//
//    #endif
// (end code)
//    With this declaration, a.c and b.c only have to include h.h to be able to access
// gflipTable, which is defined in t.c this way:
// (start code)
//    #include "h.h"
//
//    // Create pixel flipping table for mode 2, at address 0x3A00
//    cpctm_createPixelFlippingTable(gflipTable, 0x3A00, M2);
// (end code)
//    Then, for instance, a.c. can make use of the table like in this example:
// (start code)
//    #include "h.h"
//
//    //.... code ....
//
//    // Draw the sprite of the main character horizontally flipped
//    pmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
//    cpct_drawSpriteHFlip_at(sprite, pmem, WIDTH, HEIGHT, gflipTable);
//
//    //.... code ....
//
// (end code)
//
#define cpctm_declarePixelFlippingTable(TABLENAME) extern const u8 TABLENAME[256]

#endif