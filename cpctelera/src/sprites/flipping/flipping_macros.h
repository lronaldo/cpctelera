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
//### SUBMODULE: flipping.macros
//#####################################################################
//### Macros used to speed up calculations required for to assist
//### flipping functions.
//#####################################################################
//

#ifndef CPCT_SPRITEFLIPPING_MACROS_H
#define CPCT_SPRITEFLIPPING_MACROS_H

//
// Macro: cpctm_spriteBottomLeftPtr
//
//    Macro that calculates a pointer to the bottom left pixel of a given sprite.
//
// C Definition:
//    #define <cpctm_spriteBottomLeftPtr> (*SP*, *X*, *Y*)
//
// Parameters:
//    (2B) SP   - Pointer to the sprite (top-left corner)
//    (1B) X    - Width of the sprite in *bytes*
//    (1B) Y    - Height of the sprite in pixels
//
// Parameter Restrictions:
//    *SP*  - Must be a 16-bits memory address pointing to the initial byte
// of an sprite array. The sprite array must be rectangular and all its bytes
// must be consecutive in memory, from top-left to bottom-right.
//    *X*   - Must be the width of the sprite in *bytes* (Beware! Not in pixels).
// For sprites having interlaced mask, you may input 2 times the width of the
// sprite for appropriate results using this function.
//    *Y*   - Must be the height of the sprite in pixels.
//
// Returns:
//    <u16>  pointer to the bottom-left byte of the sprite in memory.
//
// Details:
//    This macro calculates the location of the initial byte of the last row 
// of a given sprite (i.e. its bottom-left byte). Some flipping functions 
// need this value to do their job (like <cpct_vflipSpriteM0>). Normally, 
// width and height of sprites are constant values. In this case, the multiplication
// needed to calculate the bottom-left byte can be done at compile-time. 
// This is what dos macro does. It calculates that multiplication at compile-time,
// transforming this calculation into a simple sum of the original pointer of
// the sprite and the offset required to point to the bottom-left pointer.
// This offset is *X* * (*Y*-1), which will be calculated at compile-time
// if *X* and *Y* are constant values.
//    Therefore, use this macro when your width and height values are constant.
// If, for any reason, you have variable sprite sizes, using this macro will
// expand into inline multiplication code, which will probably be slower and
// bigger. 
//
// Known issues:
//    * This is a C-language macro. It cannot be called or used from assembly code.
//
#define cpctm_spriteBottomLeftPtr(SP, X, Y)		(SP) + ((X) * (Y-1))

//
// Macro: cpctm_vflipSpriteMasked
//
//    Vertically flips a masked sprite
//
// C Definition:
//    #define <cpctm_vflipSpriteMasked> (W, H, SPBL, SP)
//
// Parameters:
//    (1B) W    - Width of the sprite in *bytes* (Without counting on the mask bytes)
//    (1B) H    - Height of the sprite in pixels
//    (2B) SPBL - Pointer to the bottom-left row of the sprite
//    (2B) SP   - Pointer to the sprite (top-left corner, first byte)
//
// Details:
//    This is a convenience macro that calls <cpct_vflipSprite> with same 
// parameters given, except for the width (*W*) that will be multiplied
// by two, to take into account mask bytes. 
//    Therefore, it is a mere front-end to call <cpct_vflipSprite>. For more
// details on parameter restrictions, issues and working details, please
// refer to <cpct_vflipSprite> documentation.
//
// Known issues:
//    * This is a C-language macro. It cannot be called or used from assembly code.
//
#define cpctm_vflipSpriteMasked(W, H, SPBL, SP)  cpct_vflipSprite(2*(W), (H), (SPBL), (SP))

#endif