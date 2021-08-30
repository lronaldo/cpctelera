//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#ifndef _SPRITE_MACROS_H
#define _SPRITE_MACROS_H

//----------------------------------------------------------------------------------------
// Title: Sprite Macros
//----------------------------------------------------------------------------------------

#include <types.h>

//
// Macro: CPCTM_PEN2PIXELPATTERN_M1
//
//    Similarly to the function <cpct_pen2pixelPatternM1>, creates 1 byte in Mode 1 
// screen pixel format containing a pattern with all 4 pixels in the same pen colour 
// as given by the argument *PEN*.
//
// C Definition:
//    #define <CPCTM_PEN2PIXELPATTERN_M1> (*PEN*)
//
// Parameters:
//    PEN - ([0-3], unsigned) Pen Colour from which create the pixel pattern
// 
// Known limitations:
//    * It does not perform any kind of checking. Values other than unsigned integers [0-3]
// will produce undefined behaviour.
//    * As this is a macro, this is designed only for CONSTANT values, to perform compile-time
// calculations. If you use it with variables, it will generate poor & slow code. Moreover, 
// generated code will be repeated if the macro is used multiple times, bloating your binary.
// Use <cpct_pen2pixelPatternM1> function for variables instead.
//
// Details:
//    This macro does the same operations the function <cpct_pen2pixelPatternM1> does, but
// with an explicit operation that can be calculated at compile-time, producing a single 
// precalculated byte in your final binary (if you use it with constant values).
//
//    For more details on this operations, consult <cpct_pen2pixelPatternM1> help.
//
// Use example:
//    This is the same example you will find in <cpct_pen2pixelPatternM1> but using
// constant values instead of variable ones. This hightlights their difference,
// (start code)
//    // Entity struct declaration
//    typedef struct {
//       u8* sprite;
//       u8 width, height;
//       // .....
//    } Entity_t;
//    
//    // ......
//    
//    // This function replaces all Red Pixels (Pen 3) by 
//    // Yellow Pixels (Pen 1) in the entity given
//    void replaceSpriteColourConstant(Entity_t* e) {
//       // Create 16-bit Replace Pattern required as parameter
//       u16 const rplcPat = 
//            256 * ((u16)CPCTM_PEN2PIXELPATTERN_M1(3))  // FindPat, Higher 8-bits
//                +       CPCTM_PEN2PIXELPATTERN_M1(1);  // InsrPat, Lower  8-bits
//    
//       // Calculate complete size of the sprite array
//       u8  const size    = e->width * e->height;
//    
//       // Replace colour OldPen with colour NewPen in 
//       // all the pixels of the sprite of entity e
//       cpct_spriteColourizeM1(rplcPat, size, e->sprite);
//    }
// (end code)
//
#define CPCTM_PEN2PIXELPATTERN_M1(PEN) \
   ((((PEN) & 1) << 7) | (((PEN) & 1) << 6) | (((PEN) & 1) << 5) | (((PEN) & 1) << 4) | \
    (((PEN) & 2) << 2) | (((PEN) & 2) << 1) | (((PEN) & 2)     ) | (((PEN) & 2) >> 1))


//
// Macro: CPCTM_PENS2PIXELPATTERNPAIR_M1
//
//    Similarly to the function <cpct_pens2pixelPatternPairM1>, creates a 16 bits 
// value in Mode 1 screen pixel format, containing two pattern with all 4 pixels in 
// the same pen colour as given by the arguments *PEN1* and *PEN2*. 
//
// C Definition:
//    #define <CPCTM_PENS2PIXELPATTERNPAIR_M1> (*PEN1*, *PEN2*)
//
// Parameters:
//    PEN1 - ([0-3], unsigned) First  Pen Colour (NewPen when used for <cpct_spriteColourizeM1>)
//    PEN2 - ([0-3], unsigned) Second Pen Colour (OldPen when used for <cpct_spriteColourizeM1>)
// 
// Known limitations:
//    * It does not perform any kind of checking. Values other than unsigned integers [0-3]
// will produce undefined behaviour.
//    * As this is a macro, this is designed only for CONSTANT values, to perform compile-time
// calculations. If you use it with variables, it will generate poor & slow code. Moreover, 
// generated code will be repeated if the macro is used multiple times, bloating your binary.
// Use <cpct_pen2pixelPatternM1> function for variables instead.
//
// Details:
//    This macro does the same operations the function <cpct_pens2pixelPatternPairM1> does, 
// but with an explicit operation that can be calculated at compile-time, producing a single 
// precalculated 16-bits value in your final binary (if you use it with constant values).
//
//    For more details on this operations, consult <cpct_pens2pixelPatternPairM1> help.
//
// Use example:
//    This is the same example you will find in <cpct_pen2pixelPatternM1> but using
// constant values instead of variable ones. This hightlights their difference,
// (start code)
//    // Entity struct declaration
//    typedef struct {
//       u8* sprite;
//       u8 width, height;
//       // .....
//    } Entity_t;
//    
//    // ......
//    
//    // This function replaces all Red Pixels (Pen 3) by 
//    // Yellow Pixels (Pen 1) in the entity given
//    void replaceSpriteColourConstant(Entity_t* e) {
//       // Create 16-bit Replace Pattern required as parameter
//       //    FindPat = 3, InsrPat = 1 (Replace Pen 3 with Pen 1)
//       u16 const rplcPat = CPCTM_PENS2PIXELPATTERNPAIR_M1(1, 3);
//    
//       // Calculate complete size of the sprite array
//       u8  const size    = e->width * e->height;
//    
//       // Replace colour OldPen with colour NewPen in 
//       // all the pixels of the sprite of entity e
//       cpct_spriteColourizeM1(rplcPat, size, e->sprite);
//    }
// (end code)
//
#define CPCTM_PENS2PIXELPATTERNPAIR_M1(PEN1, PEN2) \
   (((u16)CPCTM_PEN2PIXELPATTERN_M1(PEN2)) << 8) | ((u16)CPCTM_PEN2PIXELPATTERN_M1(PEN1))


#endif