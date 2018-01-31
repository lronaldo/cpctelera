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
//### MODULE: Sprites                                               ###
//#####################################################################
//### This module contains several functions and routines to manage ###
//### sprites and video memory in an Amstrad CPC environment.       ###
//#####################################################################
//

#ifndef CPCT_SPRITES_H
#define CPCT_SPRITES_H

#include <types.h>
#include "sprite_types.h"
#include "transparency_table_macros.h"
#include "drawToSpriteBuffer/drawToSpriteBuffer.h"
#include "screenToSprite/screenToSprite.h"
#include "drawTile/drawTile.h"
#include "flipping/flipping.h"
#include "blending/blending.h"

// Functions to transform firmware colours for a group of pixels into a byte in screen pixel format
extern   u8 cpct_px2byteM0 (u8 px0, u8 px1) __z88dk_callee;
extern   u8 cpct_px2byteM1 (u8 px0, u8 px1, u8 px2, u8 px3);

//
// Macro: cpctm_px2byteM0
//
//    Macro that transforms 2 pixel colour values [0-15] into a byte value in the video memory pixel format for Mode 0.
//
// C Definition:
//    #define <cpctm_px2byteM0> (*X*, *Y*)
//
// Parameters:
//    (1B) X    - X Palette Index colour value for left pixel (pixel 0) [0-15]
//    (1B) Y    - Y Palette Index colour value for left pixel (pixel 1) [0-15]
//
// Parameter Restrictions:
//    See <cpct_px2byteM0>
//
// Returns:
//    u8	byte with *X* and *Y* colour information in screen pixel format.
//
// Details:
//    This macro does the same calculation than the function <cpct_px2byteM0>. However,
// as it is a macro, if all 2 parameters (*X*, *Y*) are constants, the calculation
// will be done at compile-time. This will free the binary from code or data, just puting in
// the result of this calculation (1 byte with the video memory pixel format for Mode 0). 
// It is highly  recommended to use this macro instead of the function <cpct_px2byteM0> when values
// involved are all constant. 
//
//    Take care of using this macro with variable values. In this latest case, the compiler 
// will generate in-place code for doing the calculation. Therefore, that will take binary
// space for the code and CPU time for the calculation. Moreover, calculation will be slower
// than if it were done using <cpct_px2byteM0> and code could be duplicated if this macro
// is used in several places. Therefore, for variable values, <cpct_px2byteM0> is recommended.
//
//    Sum up of recommendations:
//    All constant values - Use this macro <cpctm_px2byteM0>
//    Any variable value  - Use the function <cpct_px2byteM0>
//
#define cpctm_px2byteM0(X, Y) (u8)(	((X & 0x01) << 6 | (X & 0x02) << 1 | (X & 0x04) << 2 | (X & 0x08) >> 3) << 1 | \
                                    ((Y & 0x01) << 6 | (Y & 0x02) << 1 | (Y & 0x04) << 2 | (Y & 0x08) >> 3) )
											
//
// Macro: cpctm_px2byteM1
//
//    Macro that transforms 4 pixel colour values [0-3] into a byte value in the video memory pixel format for Mode 1.
//
// C Definition:
//    #define <cpctm_px2byteM1> (*A*, *B*, *C*, *D*)
//
// Parameters:
//    (1B) A    - A Palette Index colour value for left pixel (pixel 0) [0-3]
//    (1B) B    - B Palette Index colour value for left pixel (pixel 1) [0-3]
//    (1B) C    - C Palette Index colour value for left pixel (pixel 3) [0-3]
//    (1B) D    - D Palette Index colour value for left pixel (pixel 4) [0-3]
//
// Parameter Restrictions:
//    See <cpct_px2byteM1>
//
// Returns:
//    u8	byte with *A*, *B*, *C* and *D* colour information in screen pixel format.
//
// Details:
//    This macro does the same calculation than the function <cpct_px2byteM1>. However,
// as it is a macro, if all 4 parameters (*A*, *B*, *C*, *D*) are constants, the calculation
// will be done at compile-time. This will free the binary from code or data, just puting in
// the result of this calculation (1 byte with the video memory pixel format for Mode 1). 
// It is highly  recommended to use this macro instead of the function <cpct_px2byteM1> when values
// involved are all constant. 
//
//    Take care of using this macro with variable values. In this latest case, the compiler 
// will generate in-place code for doing the calculation. Therefore, that will take binary
// space for the code and CPU time for the calculation. Moreover, calculation will be slower
// than if it were done using <cpct_px2byteM1> and code could be duplicated if this macro
// is used in several places. Therefore, for variable values, <cpct_px2byteM1> is recommended.
//
//    Sum up of recommendations:
//    All constant values - Use this macro <cpctm_px2byteM1>
//    Any variable value  - Use the function <cpct_px2byteM1>
//
#define cpctm_px2byteM1(A, B, C, D) (u8)(	((A & 0x01) << 4 |  (A & 0x02) >> 1) << 3 | \
                                          ((B & 0x01) << 4 |  (B & 0x02) >> 1) << 2 | \
                                          ((C & 0x01) << 4 |  (C & 0x02) >> 1) << 1 | \
                                          ((D & 0x01) << 4 |  (D & 0x02) >> 1) )

// Sprite and box drawing functions
extern void cpct_drawSolidBox        (void *memory, u8 colour_pattern, u8 width, u8 height);
extern void cpct_drawSprite          (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteMasked    (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteMaskedAlignedTable(const void *psprite, void* pvideomem, 
                                              u8 height, u8 width, const void* pmasktable) __z88dk_callee;

#endif
