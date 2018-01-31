//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2017 Bouche Arnaud
//  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
//### SUBMODULE: drawToSpriteBuffer
//#####################################################################
//### This module contains routines to draw sprites inside another
//### sprite's array. This lets using destination sprite as a sort of
//### backbuffer, or compositing sprites.
//#####################################################################
//

#ifndef CPCT_DRAWTOSPRITEBUFFER_H
#define CPCT_DRAWTOSPRITEBUFFER_H

// Sprite to Sprite-Buffer Drawing Functions
extern void cpct_drawToSpriteBuffer                   (u16 buffer_width, void* buffer, u8 width, u8 height, 
                                                      void* sprite) __z88dk_callee;
extern void cpct_drawToSpriteBufferMasked             (u16 buffer_width, void* buffer, u8 width, u8 height, 
                                                      void* sprite) __z88dk_callee;
extern void cpct_drawToSpriteBufferMaskedAlignedTable (u16 buffer_width, void* buffer, u8 width, u8 height,
                                                       void* sprite, u8* mask_table) __z88dk_callee;

//
// Title: Macros (C)
//
//    Useful sprite-buffer related macros designed to be used in your C programs
//

//
// Macro: cpctm_spriteBufferPtr
//
//    Calculates a pointer inside a sprite-buffer based on (X,Y) coordinates relative
// to the sprite-buffer space, and WIDTH of the sprite-buffer.
//
// C Definition:
//    #define <cpctm_spriteBufferPtr> (*SPRITE*, *WIDTH*, *X*, *Y*) 
//
// ASM Call:
//    For asm programs, please refer to <cpctm_spriteBufferPtr_asm>
//
// Parameters:
//    SPRITE    - Pointer to the start of the Sprite-Buffer array
//    WIDTH     - Width in bytes of each line of the Sprite-Buffer Array
// care with this value not to overlap other parts of the code.
//    X         - X coordinate of the point inside the sprite to be calculated
//    Y         - Y coordinate of the point inside the sprite to be calculated
// 
// Known limitations:
//    * This macro will produce no-code when used with constant values. If any of
// the given values is a variable, it will procuce calculation code. This code
// will be translated by C-compiler into ASM, and may be slow.
//
#define cpctm_spriteBufferPtr(SPRITE, WIDTH, X, Y) ((SPRITE) + (WIDTH)*(Y) + (X))

#endif