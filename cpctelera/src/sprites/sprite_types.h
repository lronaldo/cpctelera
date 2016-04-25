//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
// Title: Sprite Types
//

#ifndef CPCT_SPRITE_TYPES_H
#define CPCT_SPRITE_TYPES_H

// 
// Enum: CPCT_BlendMode
//
//    Enumerates all blending modes for <cpct_drawSpriteBlended>. 
//
// Description:
//    These blend operations are common byte operations that the processor
// does between two values. The <cpct_drawSpriteBlended> function blends
// the sprite with the background (present contents of the video memory)
// byte by byte. It takes 1 byte from video memory (background) and 1 byte
// from the sprite array (data), does the selected operation with both 
// (XOR, OR, AND, etc) and puts the result in screen video memory, where
// it got the first byte from the background.
//
//    All possible blending modes are implemented as 1-byte Z80 operations.
// The name of every enumerate value include the mnemonic of the operation
// that will be performed (XOR, AND, OR, ADD...) between both bytes 
// (background and sprite data). Take into account that pixel values refer
// to palette indices (0-1, 0-3 or 0-15, depending on graphics mode). Final
// result of operations will depend on how you select your palette colours.
//
// Modes available:
// (start code)
//    %======================================================%
//    | Constant       | Value | Blending operation          |
//    |------------------------------------------------------|
//    | CPCT_BLEND_XOR |  0xAE | = background ^ data         |
//    | CPCT_BLEND_AND |  0xA6 | = background & data         |
//    | CPCT_BLEND_OR  |  0xB6 | = background | data         |
//    | CPCT_BLEND_ADD |  0x86 | = background + data         |
//    | CPCT_BLEND_ADC |  0x8E | = background + data + Carry |
//    | CPCT_BLEND_SBC |  0x9E | = background - data - Carry |
//    | CPCT_BLEND_SUB |  0x96 | = background - data         |
//    | CPCT_BLEND_LDI |  0x7E | = data                      |
//    | CPCT_BLEND_NOP |  0x00 | = background                |
//    %======================================================%
// (end code)
// * Background = bytes of data read from screen video memory (where background lies)
// * Data       = bytes of data read from the sprite
// * Carry      = Carry bits from previous additions and subtractions
//
typedef enum {
     CPCT_BLEND_XOR = 0xAE
   , CPCT_BLEND_AND = 0xA6
   , CPCT_BLEND_OR  = 0xB6
   , CPCT_BLEND_ADD = 0x86
   , CPCT_BLEND_ADC = 0x8E
   , CPCT_BLEND_SBC = 0x9E
   , CPCT_BLEND_SUB = 0x96
   , CPCT_BLEND_LDI = 0x7E
   , CPCT_BLEND_NOP = 0x00
} CPCT_BlendMode;

#endif
