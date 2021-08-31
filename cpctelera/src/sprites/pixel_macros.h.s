;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU Lesser General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU Lesser General Public License for more details.
;;
;;  You should have received a copy of the GNU Lesser General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------

;;-------------------------------------------------------------------------------
;; Title: Pixel Macros (ASM)
;;-------------------------------------------------------------------------------

;; Macro: CPCTM_PEN2PIXELPATTERN_M1_ASM
;;
;;    Similarly to the function <cpct_pen2pixelPatternM1>, creates 1 byte in Mode 1 
;; screen pixel format containing a pattern with all 4 pixels in the same pen colour 
;; as given by the argument *PEN*.
;;
;; ASM Definition:
;;    .macro <CPCTM_PEN2PIXELPATTERN_M1_ASM> *SYM*, *PEN*
;;
;; Parameters:
;;    SYM - Name of a symbol to assign the resulting value of the calculation
;;    PEN - ([0-3], unsigned) Pen Colour from which create the pixel pattern
;; 
;; Known limitations:
;;    * It does not perform any kind of checking. Values other than unsigned integers [0-3]
;; will produce undefined behaviour.
;;    * As this is an assembly macro, it will only work with constant values or symbols. It
;; will produce assembly errors when used with other kind of values.
;;
;; Details:
;;    This macro does the same operations the function <cpct_pen2pixelPatternM1> does, but
;; with an explicit operation that can be calculated at compile-time, producing a single 
;; precalculated byte in your final binary (if you use it with constant values).
;;
;;    For more details on this operations, consult <cpct_pen2pixelPatternM1> help.
;;
;; Use example:
;; (start code)
;;    ;// Simple function that changes the Skin Colour 
;;    ;// of a given Alien Sprite into Cyan (Alien Skin is yellow normally)  
;;    ;// HL = Pointer to Alien Sprite
;;    ;// BC = Size of Alien Sprite
;;    changeAlienSkinColourToCyan:
;;       CPCTM_PEN2PIXELPATTERN_M1_ASM OldPen, 1 ;// Produces 'OldPen = 0xF0'
;;       CPCTM_PEN2PIXELPATTERN_M1_ASM NewPen, 2 ;// Produces 'NewPen = 0x0F'
;;       ld    de, #(OldPen << 8) | NewPen
;;       call  cpct_spriteColourizeM1_asm
;;       ret
;;    
;;    ;// Simple function that restores the Skin Colour 
;;    ;// of a given Alien Sprite to yellow (after being changed to Cyan)  
;;    ;// HL = Pointer to Alien Sprite
;;    ;// BC = Size of Alien Sprite
;;    restoreAlienSkinColourToYellow:
;;       CPCTM_PEN2PIXELPATTERN_M1_ASM OldPen, 2 ;// Produces 'OldPen = 0x0F'
;;       CPCTM_PEN2PIXELPATTERN_M1_ASM NewPen, 1 ;// Produces 'NewPen = 0xF0'
;;       ld    de, #(OldPen << 8) | NewPen
;;       call  cpct_spriteColourizeM1_asm
;;       ret
;; (end code)
;;
.mdelete CPCTM_PEN2PIXELPATTERN_M1_ASM
.macro CPCTM_PEN2PIXELPATTERN_M1_ASM _Sym, _Pen
   _Sym = ((((_Pen) & 1) << 7) | (((_Pen) & 1) << 6) | (((_Pen) & 1) << 5) | (((_Pen) & 1) << 4) | (((_Pen) & 2) << 2) | (((_Pen) & 2) << 1) | (((_Pen) & 2)     ) | (((_Pen) & 2) >> 1))
.endm  

;;
;; Macro: CPCTM_PENS2PIXELPATTERNPAIR_M1_ASM
;;
;;    Similarly to the function <cpct_pens2pixelPatternPairM1>, creates a 16 bits 
;; value in Mode 1 screen pixel format, containing two pattern with all 4 pixels in 
;; the same pen colour as given by the arguments *OldPen* and *NewPen*. 
;;
;; C Definition:
;;    .macro <CPCTM_PENS2PIXELPATTERNPAIR_M1_ASM> *SYM*, *OldPen*, *NewPen*
;;
;; Parameters:
;;    SYM    - Name of a symbol to assign the resulting value of the calculation
;;    OldPen - ([0-3], unsigned) Pen Colour to be found and replaced in <cpct_spriteColourizeM1>
;;    NewPen - ([0-3], unsigned) New Pen Colour to be inserted in <cpct_spriteColourizeM1>
;; 
;; Known limitations:
;;    * It does not perform any kind of checking. Values other than unsigned integers [0-3]
;; will produce undefined behaviour.
;;    * As this is a macro, this is designed only for CONSTANT values, to perform compile-time
;; calculations. If you use it with variables, it will generate poor & slow code. Moreover, 
;; generated code will be repeated if the macro is used multiple times, bloating your binary.
;; Use <cpct_pen2pixelPatternM1> function for variables instead.
;;
;; Details:
;;    This macro does the same operations the function <cpct_pens2pixelPatternPairM1> does, 
;; but with an explicit operation that can be calculated at compile-time, producing a single 
;; precalculated 16-bits value in your final binary (if you use it with constant values).
;;
;;    For more details on this operations, consult <cpct_pens2pixelPatternPairM1> help.
;;
;; Use example:
;; (start code)
;; ;// Simple function that changes the Skin Colour 
;; ;// of a given Alien Sprite into Cyan (2) (Alien Skin is yellow (1) normally)  
;; ;// HL = Pointer to Alien Sprite
;; ;// BC = Size of Alien Sprite
;; changeAlienSkinColourToCyan:
;;    CPCTM_PENS2PIXELPATTERNPAIR_M1_ASM rplcPat, 1, 2   ;// Change Pen 1 into Pen 2
;;    ld    de, #rplcPat                                 ;// rplcPat = 0xF00F
;;    call  cpct_spriteColourizeM1_asm
;;    ret
;;
;; ;// Simple function that restores the Skin Colour 
;; ;// of a given Alien Sprite to yellow (1) (after being changed to Cyan (2) )
;; ;// HL = Pointer to Alien Sprite
;; ;// BC = Size of Alien Sprite
;; restoreAlienSkinColourToYellow:
;;    CPCTM_PENS2PIXELPATTERNPAIR_M1_ASM rplcPat, 2, 1   ;// Change Pen 2 into Pen 1
;;    ld    de, #rplcPat                                 ;// rplcPat = 0x0FF0
;;    call  cpct_spriteColourizeM1_asm
;;    ret
;; (end code)
;;
.mdelete CPCTM_PENS2PIXELPATTERNPAIR_M1_ASM
.macro CPCTM_PENS2PIXELPATTERNPAIR_M1_ASM _Sym, _OldPen, _NewPen
   CPCTM_PEN2PIXELPATTERN_M1_ASM _Sym'__h, _OldPen
   CPCTM_PEN2PIXELPATTERN_M1_ASM _Sym'__l, _NewPen
   _Sym = (_Sym'__h << 8) | _Sym'__l
.endm  
