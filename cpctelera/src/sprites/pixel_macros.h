//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2021 Arnaud Bouche (@Arnaud6128)
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
//### HEADER: Pixel Macros                                          ###
//#####################################################################
//### Contains macros that transform pixel colours into screen pixel###
//### format bitpatterns, to be used at compile time.               ###
//#####################################################################
//

//----------------------------------------------------------------------------------------
// Title: Pixel Macros (C)
//----------------------------------------------------------------------------------------

#ifndef PIXEL_MACROS_H
#define PIXEL_MACROS_H

//
// Macro: CPCTM_PEN2PIXELPATTERN_M0
//
//    Similarly to the function <cpct_pen2pixelPatternM0>, creates 1 byte in Mode 0 
// screen pixel format containing a pattern with all 2 pixels in the same pen colour 
// as given by the argument *PEN*.
//
// C Definition:
//    #define <CPCTM_PEN2PIXELPATTERN_M0> (*PEN*)
//
// Parameters:
//    PEN - ([0-15], unsigned) Pen Colour from which create the pixel pattern
// 
// Known limitations:
//    * It does not perform any kind of checking. Values other than unsigned integers [0-15]
// will produce undefined behaviour.
//    * As this is a macro, this is designed only for CONSTANT values, to perform compile-time
// calculations. If you use it with variables, it will generate poor & slow code. Moreover, 
// generated code will be repeated if the macro is used multiple times, bloating your binary.
// Use <cpct_pen2pixelPatternM0> function for variables instead.
//    * *Beware!* In some versions of SDCC compiler it could even generate bugged assembly code
// if you use this macro with variables. It works without problem with constants in compile-time
// though. (Failed under SDCC 3.6.8).
//
// Details:
//    This macro does the same operations the function <cpct_pen2pixelPatternM0> does, but
// with an explicit operation that can be calculated at compile-time, producing a single 
// precalculated byte in your final binary (if you use it with constant values).
//
//    For more details on this operations, consult <cpct_pen2pixelPatternM0> help.
//
// Use example:
//    This is the same example you will find in <cpct_pen2pixelPatternM0> but using
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
//            256 * ((u16)CPCTM_PEN2PIXELPATTERN_M0(3))  // FindPat, Higher 8-bits
//                +       CPCTM_PEN2PIXELPATTERN_M0(1);  // InsrPat, Lower  8-bits
//    
//       // Calculate complete size of the sprite array
//       u8  const size    = e->width * e->height;
//    
//       // Replace colour OldPen with colour NewPen in 
//       // all the pixels of the sprite of entity e
//       cpct_spriteColourizeM0(rplcPat, size, e->sprite);
//    }
// (end code)
//
#define CPCTM_PEN2PIXELPATTERN_M0(PEN) \
    (u8)( (((PEN) & 0x01) << 6 | ((PEN) & 0x02) << 1 | ((PEN) & 0x04) << 2 | ((PEN) & 0x08) >> 3) << 1 | \
          (((PEN) & 0x01) << 6 | ((PEN) & 0x02) << 1 | ((PEN) & 0x04) << 2 | ((PEN) & 0x08) >> 3) )

//
// Macro: CPCTM_PENS2PIXELPATTERNPAIR_M0
//
//    Similarly to the function <cpct_pens2pixelPatternPairM0>, creates a 16 bits 
// value in Mode 0 screen pixel format, containing two pattern with all 2 pixels in 
// the same pen colour as given by the arguments *OldPen* and *NewPen*. 
//
// C Definition:
//    #define <CPCTM_PENS2PIXELPATTERNPAIR_M0> (*OldPen*, *NewPen*)
//
// Parameters:
//    OldPen - ([0-15], unsigned) First  Pen Colour (NewPen when used for <cpct_spriteColourizeM0>)
//    NewPen - ([0-15], unsigned) Second Pen Colour (OldPen when used for <cpct_spriteColourizeM0>)
// 
// Known limitations:
//    * It does not perform any kind of checking. Values other than unsigned integers [0-15]
// will produce undefined behaviour.
//    * As this is a macro, this is designed only for CONSTANT values, to perform compile-time
// calculations. If you use it with variables, it will generate poor & slow code. Moreover, 
// generated code will be repeated if the macro is used multiple times, bloating your binary.
// Use <cpct_pen2pixelPatternM0> function for variables instead.
//    * *Beware!* In some versions of SDCC compiler it could even generate bugged assembly code
// if you use this macro with variables. It works without problem with constants in compile-time
// though. (Failed under SDCC 3.6.8).
//
// Details:
//    This macro does the same operations the function <cpct_pens2pixelPatternPairM0> does, 
// but with an explicit operation that can be calculated at compile-time, producing a single 
// precalculated 16-bits value in your final binary (if you use it with constant values).
//
//    For more details on this operations, consult <cpct_pens2pixelPatternPairM0> help.
//
// Use example:
//    This is the same example you will find in <cpct_pen2pixelPatternM0> but using
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
//       u16 const rplcPat = CPCTM_PENS2PIXELPATTERNPAIR_M0(3, 1);
//    
//       // Calculate complete size of the sprite array
//       u8  const size    = e->width * e->height;
//    
//       // Replace colour OldPen with colour NewPen in 
//       // all the pixels of the sprite of entity e
//       cpct_spriteColourizeM0(rplcPat, size, e->sprite);
//    }
// (end code)
//
#define CPCTM_PENS2PIXELPATTERNPAIR_M0(OldPen, NewPen) \
   (u16)(((u16)CPCTM_PEN2PIXELPATTERN_M0(OldPen)) << 8) | ((u16)CPCTM_PEN2PIXELPATTERN_M0(NewPen))

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
//    * *Beware!* In some versions of SDCC compiler it could even generate bugged assembly code
// if you use this macro with variables. It works without problem with constants in compile-time
// though. (Failed under SDCC 3.6.8).
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
   (u8)((((PEN) & 1) << 7) | (((PEN) & 1) << 6) | (((PEN) & 1) << 5) | (((PEN) & 1) << 4) | \
        (((PEN) & 2) << 2) | (((PEN) & 2) << 1) | (((PEN) & 2)     ) | (((PEN) & 2) >> 1))

//
// Macro: CPCTM_PENS2PIXELPATTERNPAIR_M1
//
//    Similarly to the function <cpct_pens2pixelPatternPairM1>, creates a 16 bits 
// value in Mode 1 screen pixel format, containing two pattern with all 4 pixels in 
// the same pen colour as given by the arguments *OldPen* and *NewPen*. 
//
// C Definition:
//    #define <CPCTM_PENS2PIXELPATTERNPAIR_M1> (*OldPen*, *NewPen*)
//
// Parameters:
//    OldPen - ([0-3], unsigned) First  Pen Colour (NewPen when used for <cpct_spriteColourizeM1>)
//    NewPen - ([0-3], unsigned) Second Pen Colour (OldPen when used for <cpct_spriteColourizeM1>)
// 
// Known limitations:
//    * It does not perform any kind of checking. Values other than unsigned integers [0-3]
// will produce undefined behaviour.
//    * As this is a macro, this is designed only for CONSTANT values, to perform compile-time
// calculations. If you use it with variables, it will generate poor & slow code. Moreover, 
// generated code will be repeated if the macro is used multiple times, bloating your binary.
// Use <cpct_pen2pixelPatternM1> function for variables instead.
//    * *Beware!* In some versions of SDCC compiler it could even generate bugged assembly code
// if you use this macro with variables. It works without problem with constants in compile-time
// though. (Failed under SDCC 3.6.8).
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
//       u16 const rplcPat = CPCTM_PENS2PIXELPATTERNPAIR_M1(3, 1);
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
#define CPCTM_PENS2PIXELPATTERNPAIR_M1(OldPen, NewPen) \
   (((u16)CPCTM_PEN2PIXELPATTERN_M1(OldPen)) << 8) | ((u16)CPCTM_PEN2PIXELPATTERN_M1(NewPen))

//
// Macro: cpctm_px2byteM0
//
//    Macro that transforms 2 pixel colour values [0-15] into a byte value 
// that codifies both colours in video memory pixel format for Mode 0.
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
//    <u8> 	byte with *X* and *Y* colour information in screen pixel format.
//
// Details:
//    This macro does the same calculation than the function <cpct_px2byteM0>. However,
// as it is a macro, if both parameters (*X*, *Y*) are constants, the calculation
// will be done at compile-time. This will free the binary from code or data, just putting in
// the result of this calculation (1 single byte in video memory pixel format for Mode 0). 
// It is highly recommended to use this macro instead of the function <cpct_px2byteM0> when 
// values involved are all constant. 
//
//    Take care of using this macro with variable values. In this latest case, the compiler 
// will generate in-place code for doing the calculation. Therefore, that will take binary
// space for the code and CPU time for the calculation. Moreover, calculation will be slower
// than if it were done using <cpct_px2byteM0> and code could be duplicated if this macro
// is used in several places. Therefore, for variable values, <cpct_px2byteM0> is recommended.
//
// Sum up of recommendations,
//    All constant values - Use this macro <cpctm_px2byteM0>
//    Any variable value  - Use the function <cpct_px2byteM0>
//
// Known issues:
//		* This is a C-language macro. It cannot be called or used from assembly code.
//
#define cpctm_px2byteM0(X, Y) (u8)( (((X) & 0x01) << 6 | ((X) & 0x02) << 1 | ((X) & 0x04) << 2 | ((X) & 0x08) >> 3) << 1 | \
                                    (((Y) & 0x01) << 6 | ((Y) & 0x02) << 1 | ((Y) & 0x04) << 2 | ((Y) & 0x08) >> 3) )

//
// Macro: cpctm_px2byteM1
//
//    Macro that transforms 4 pixel colour values [0-3] into a byte value 
// that codifies both colours in video memory pixel format for Mode 1.
//
// C Definition:
//    #define <cpctm_px2byteM1> (*A*, *B*, *C*, *D*)
//
// Parameters:
//    (1B) A    - A Palette Index colour value for left pixel (pixel 0) [0-3]
//    (1B) B    - B Palette Index colour value for left pixel (pixel 1) [0-3]
//    (1B) C    - C Palette Index colour value for left pixel (pixel 2) [0-3]
//    (1B) D    - D Palette Index colour value for left pixel (pixel 3) [0-3]
//
// Parameter Restrictions:
//    See <cpct_px2byteM1>
//
// Returns:
//    <u8>	byte with *A*, *B*, *C* and *D* colour information in screen pixel format.
//
// Details:
//    This macro does the same calculation than the function <cpct_px2byteM1>. However,
// as it is a macro, if all 4 parameters (*A*, *B*, *C*, *D*) are constants, the calculation
// will be done at compile-time. This will free the binary from code or data, just putting in
// the result of this calculation (1 single byte in video memory pixel format for Mode 1). 
// It is highly recommended to use this macro instead of the function <cpct_px2byteM1> when values
// involved are all constant. 
//
//    Take care of using this macro with variable values. In this latest case, the compiler 
// will generate in-place code for doing the calculation. Therefore, that will take binary
// space for the code and CPU time for the calculation. Moreover, calculation will be slower
// than if it were done using <cpct_px2byteM1> and code could be duplicated if this macro
// is used in several places. Therefore, for variable values, <cpct_px2byteM1> is recommended.
//
// Sum up of recommendations,
//    All constant values - Use this macro <cpctm_px2byteM1>
//    Any variable value  - Use the function <cpct_px2byteM1>
//
// Known issues:
//		* This is a C-language macro. It cannot be called or used from assembly code.
//
#define cpctm_px2byteM1(A, B, C, D) (u8)( (((A) & 0x01) << 4 |  ((A) & 0x02) >> 1) << 3 | \
                                          (((B) & 0x01) << 4 |  ((B) & 0x02) >> 1) << 2 | \
                                          (((C) & 0x01) << 4 |  ((C) & 0x02) >> 1) << 1 | \
                                          (((D) & 0x01) << 4 |  ((D) & 0x02) >> 1) )

#endif
