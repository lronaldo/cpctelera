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
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_pens2pixelPatternPairM1
;;
;;    Returns 16 bits in Mode 1 screen pixel format, containing two pattern with
;; all 4 pixels in the same pen colour as given by the arguments *OldPen* and *NewPen*.
;;
;; C Definition:
;;    #define <cpct_pens2pixelPatternPairM1> (*OldPen*, *NewPen*)
;;
;;    <u16> cpct_pens2pixelPatternPairM1_real (<u8> *NewPen*, <u8> *OldPen*) __z88dk_callee
;;
;;    Note - To make arguments clearer and easier to use, <cpct_pens2pixelPatternPairM1> is
;; defined as a macro. The actual function has suffix '_real' and its arguments are inverted.
;;
;; Assembly Call (Input parameters on Registers):
;;    > call cpct_pens2pixelPatternPairM1_asm
;;
;; Input Parameters (2 bytes):
;;    (1B E) NewPen - Pen colour to be inserted when used for <cpct_spriteColourizeM1>.
;;    (1B D) OldPen - Pen colour to be found and replaced when used for <cpct_spriteColourizeM1>.
;;
;; Return value:
;;    A 16-bits unsigned containing 2 bytes, each one with a 4-pixel-byte pattern in 
;; mode 1 screen pixel format, with its 4 pixels coloured same as the parameters 
;; (*OldPen* for higher byte, *NewPen* for lower byte)
;;
;;    C-bindings   - 2 bytes (in HL)
;;    ASM-bindings - 2 bytes (in DE)
;;
;; Parameter Restrictions:
;;    * *OldPen*/*NewPen* ([0-3], 8 bits, unsigned). Values outside the [0-3] range 
;; will produce undefined behaviour. 
;; 
;; Known limitations:
;;    * Parameter boundaries are not checked. Providing parameters out of range 
;; will result in undefined behaviour.
;;    * This function is to be used with variable parameters. For constant 
;; parameters it is better to use the macro version <CPCTM_PENS2PIXELPATTERNPAIR_M1> 
;; to save CPU time and bytes.
;;
;; Details:
;;    This function receives 2 pen colours as parameters (Mode 1, [0-3]) and uses 
;; them to create two bytes in mode 1 screen pixel format, each one with its 4 pixels 
;; of the corresponding pen colour provided. 
;; 
;;    This function works similar to <cpct_pen2pixelPatternM1> but it is faster 
;; for calculating a pair of patterns, which is usually required for functions like
;; <cpct_spriteColourizeM1>. The value it returns can be used directly by
;; <cpct_spriteColourizeM1>. 
;;
;;    For more details on how this function performs, please read the documentation
;; of the function <cpct_pen2pixelPatternM1>.
;;    
;;    The following example shows how this function can be used to save bytes and
;; time when used in conjunction with <cpct_spriteColourizeM1>,
;; (start code)
;;    // Entity struct declaration
;;    struct Entity_t {
;;       u8* sprite;
;;       u8 width, height;
;;       // .....
;;    };
;;
;;    // ......
;;
;;    // This function replaces all pixels of a given colour (OldPen) inside the
;;    // sprite of an entity by another given colour (NewPen)
;;    void replaceSpriteColour(Entity_t* e, u16 OldPen, u16 NewPen) {
;;       // Get 4-pixels-byte patterns for OldPen and NewPen 
;;       // in a single u16 (replace Pattern)
;;       u16 const rplcPat = cpct_pens2pixelPatternPairM1(OldPen, NewPen);
;;       // Calculate complete size of the sprite array
;;       u8  const size    = e->width * e->height;
;;       // Replace colour OldPen with colour NewPen in 
;;       // all the pixels of the sprite of entity e
;;       cpct_spriteColourizeM1(rplcPat, size, e->sprite)
;;    }
;; (end code)
;;
;; Destroyed Register values:
;;    F, BC, DE, HL
;;
;; Required memory:
;;    C-bindings   - 18 bytes
;;    ASM-bindings - 14 bytes 
;;
;; Time Measures: 
;; (start code)
;;  --------------------------------------------
;;  |    Case    | microSecs (us) | CPU Cycles |
;;  --------------------------------------------
;;  | Any        |       32       |    128     |
;;  --------------------------------------------
;;  | ASM-Saving |      -11       |    -44     |
;;  --------------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_pen2fourPixelM1_table

   ;; Get Pixel Pattern for NewPen (E) -> Put it temporarily in A
   ld    bc, #cpct_pen2fourPixelM1_table  ;; [3] BC Points to the start of the Replace Colours Pattern Conversion Array
   ld     h, #0                           ;; [2] / HL = E = NewPen (H = 0, L = E)
   ld     l, e                            ;; [1] \
   add   hl, bc                           ;; [3] HL += BC // HL now points to the Colour Pattern for the NewPen
   ld     e, (hl)                         ;; [1] E = Pixel Pattern for NewPen
   ;; Get Pixel Pattern for OldPen (E) -> Put it in E
   ld     h, #0      ;; [2] / HL = D = OldPen (H = 0, L = D)
   ld     l, d       ;; [1] \ 
   add   hl, bc      ;; [3] ;; [3] HL += BC // HL now points to the Colour Pattern for the OldPen
   ld     d, (hl)    ;; [2] D = Pixel Pattern for OldPen

   ;; Return managed at binding code
