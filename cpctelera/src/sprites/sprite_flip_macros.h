//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#ifndef _CPCT_FLIPMACROS_H_
#define _CPCT_FLIPMACROS_H_

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// File: Useful Macros
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// 
// Macro: cpct_hflipSpriteMasked
//   Horizontally flips a sprite with interlaced mask, left-to-right and viceversa
//
// C definition:
//   #define <cpct_hflipSpriteMasked> (MODE, Sprite, Width, Height)
//
// Input parameters:
//    MODE   - Selector for Graphics mode. Valid values are { M0, M1, M2 } (Always in capitals).
//    Sprite - Pointer to the sprite
//    Width  - Width of the sprite *in bytes* (without taking into account mask)
//    Height - Height of the sprite in bytes or pixels (both quantities should be equal)
//
// Known limitations:
//   * This macro uses functions that will not work from ROM, because they use
// self-modifying code.
//   * This is a C macro, and it is not possible to use it from Assembly. If you 
// wanted to use it from assembly, call the equivalent cpct_hflipSpriteXX function
// with double width for the sprite (as it has pairs of bytes with colours and mask
// instead of only colours). 
//
// Use example:
//    This example draws a car with interlaced mask over a road. The car is flipped 
// along with its mask when it turns (when the users presses reverse key)
// (start code)
//   // Draws a car over a road in a required direction in mode 1
//   void drawCar(u8 x, u8 y) {
//      u8 pmem;
//
//      // Check if the user has pressed reverse key
//      if(cpct_isKeyPressed(Key_Space))
//         cpct_hflipSpriteMasked(M1, carSprite, 8, 8);
//
//      // Draw the car on the road, at (x,y) location
//      pmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
//      cpct_drawSprite(characterSprite, pmem, 8, 8);
//   }
// (end code)
//
#define cpct_hflipSpriteMasked(MODE, SP, W, H) cpct_hflipSprite ## MODE (2*(W),(H),(SP))

// 
// Macro: cpct_hflipSpriteMasked_r
//   Horizontally flips a sprite with interlaced mask, left-to-right and viceversa. 
// (ROM-friendly version)
//
// C definition:
//   #define <cpct_hflipSpriteMasked_r> (MODE, Sprite, Width, Height)
//
// Input parameters:
//    MODE   - Selector for Graphics mode. Valid values are { M0, M1, M2 } (Always in capitals).
//    Sprite - Pointer to the sprite
//    Width  - Width of the sprite *in bytes* (without taking into account mask)
//    Height - Height of the sprite in bytes or pixels (both quantities should be equal)
//
// Known limitations:
//   * This is a C macro, and it is not possible to use it from Assembly. If you 
// wanted to use it from assembly, call the equivalent cpct_hflipSpriteXX function
// with double width for the sprite (as it has pairs of bytes with colours and mask
// instead of only colours). 
//
// Use example:
//    This example draws a car with interlaced mask over a road. The car is flipped 
// along with its mask when it turns (when the users presses reverse key)
// (start code)
//   // Draws a car over a road in a required direction in mode 1
//   void drawCar(u8 x, u8 y) {
//      u8 pmem;
//
//      // Check if the user has pressed reverse key
//      if(cpct_isKeyPressed(Key_Space))
//         cpct_hflipSpriteMasked_r(M1, carSprite, 8, 8);
//
//      // Draw the car on the road, at (x,y) location
//      pmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
//      cpct_drawSprite(characterSprite, pmem, 8, 8);
//   }
// (end code)
//
#define cpct_hflipSpriteMasked_r(MODE, SP, W, H) cpct_hflipSprite ## MODE ## _r((SP), 2*(W),(H))

#endif