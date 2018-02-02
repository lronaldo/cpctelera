;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_setSpriteColourizeM1
;;
;;    Sets the concrete colour that will be replaced by <cpct_spriteColourizeM1> when called.
;;
;; C Definition:
;;    void <cpct_setSpriteColourizeM1> (<u8> *oldColor*, <u8> *newColor*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (1B L )  oldColor     - Colour to be replaced (Palette Index, 0-4)
;;  (1B H )  newColor     - New colour (Palette Index, 0-4)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setSpriteColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *oldColor* must be the palette index of the colour to be replaced (0 to 4).
;;  * *newColor* must be the palette index of the new colour (0 to 4).
;;
;; Known limitations:
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    <TODO>
;;
;; Destroyed Register values: 
;;    A, BC, HL
;;
;; Required memory:
;;     C-bindings - 37 bytes
;;   ASM-bindings - 35 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;   Any      |          68            |        272
;; ----------------------------------------------------------------
;; Asm saving |          -9            |        -36
;; ----------------------------------------------------------------
;; (end code)
;;
;; Credits:
;;    Original routine optimized by @Docent and discussed in CPCWiki :
;; http://www.cpcwiki.eu/forum/programming/cpctelera-colorize-sprite/
;;
;; Thanks to all of them for their help and support.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl dc_mode1_ct                ;; Look-Up-Table to convert Palette Indexes to 2-bits pixel 0 screen format patterns
.include "macros/cpct_luts.h.s"   ;; Macros to easily access the Look-Up-Table
;; Symbols for placeholders inside the Colourize function
.globl cpct_spriteColourizeM1_px3_newval
.globl cpct_spriteColourizeM1_px2_newval
.globl cpct_spriteColourizeM1_px1_newval
.globl cpct_spriteColourizeM1_px0_newval

.globl cpct_spriteColourizeM1_px3_oldval
.globl cpct_spriteColourizeM1_px2_oldval
.globl cpct_spriteColourizeM1_px1_oldval
.globl cpct_spriteColourizeM1_px0_oldval

;; Symbols for placeholders inside the Masked Colourize function
.globl cpct_spriteMaskedColourizeM1_px3_newval
.globl cpct_spriteMaskedColourizeM1_px2_newval
.globl cpct_spriteMaskedColourizeM1_px1_newval
.globl cpct_spriteMaskedColourizeM1_px0_newval

.globl cpct_spriteMaskedColourizeM1_px3_oldval
.globl cpct_spriteMaskedColourizeM1_px2_oldval
.globl cpct_spriteMaskedColourizeM1_px1_oldval
.globl cpct_spriteMaskedColourizeM1_px0_oldval

 ;; Use Look-Up-Table to convert palette index colour to screen pixel format
 ;; This conversion is for the Pixel 1 into the four pixels each byte has in mode 1 [0,1,2,3]
 ;; Therefore, only bits 0xxx1xxx will be produced.

 ;; Convert newColour to pixel format 
 ld a, h                                         ;; [1]  A = H new colour index
 cpctm_lutget8 dc_mode1_ct, b, c                 ;; [10] Get from Look-Up-Table dc_mode0_ct[BC + A]
 ld (cpct_spriteColourizeM1_px3_newval), a       ;; [4]  Write Pixel 3 format (0xxx 1xxx) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px3_newval), a ;; [4]  Write Pixel 3 format (0xxx 1xxx) into Masked Colourize function code
 
 rrca                                            ;; [1]  Convert Pixel 2 format to Pixel 0, shifting bits to the right
 ld (cpct_spriteColourizeM1_px2_newval), a       ;; [4]  Write Pixel 2 format (x0xx x1xx) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px2_newval), a ;; [4]  Write Pixel 2 format (x0xx x1xx) into Masked Colourize function code
 
 rrca                                            ;; [1]  Convert Pixel 1 format to Pixel 0, shifting bits to the right
 ld (cpct_spriteColourizeM1_px1_newval), a       ;; [4]  Write Pixel 1 format (xx0x xx1x) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px1_newval), a ;; [4]  Write Pixel 1 format (xx0x xx1x) into Masked Colourize function code
  
 rrca                                            ;; [1]  Convert Pixel 1 format to Pixel 0, shifting bits to the right
 ld (cpct_spriteColourizeM1_px0_newval), a       ;; [4]  Write Pixel 0 format (xxx0 xxx1) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px0_newval), a ;; [4]  Write Pixel 0 format (xxx0 xxx1) into Masked Colourize function code
 
 ;; Convert oldColour to pixel format 
 ld a, l                                         ;; [1]  A = L old colour index
 cpctm_lutget8 dc_mode1_ct, b, c                 ;; [10] Get from Look-Up-Table dc_mode0_ct[BC + A]
 ld (cpct_spriteColourizeM1_px3_oldval), a       ;; [4]  Write Pixel 3 format (0xxx 1xxx) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px3_oldval), a ;; [4]  Write Pixel 3 format (0xxx 1xxx) into Masked Colourize function code
  
 rrca                                            ;; [1]  Convert Pixel 2 format to Pixel 0, shifting bits to the right
 ld (cpct_spriteColourizeM1_px2_oldval), a       ;; [4]  Write Pixel 2 format (x0xx x1xx) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px2_oldval), a ;; [4]  Write Pixel 2 format (x0xx x1xx) into Masked Colourize function code
 
 rrca                                            ;; [1]  Convert Pixel 1 format to Pixel 0, shifting bits to the right
 ld (cpct_spriteColourizeM1_px1_oldval), a       ;; [4]  Write Pixel 1 format (xx0x xx1x) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px1_oldval), a ;; [4]  Write Pixel 1 format (xx0x xx1x) into Masked Colourize function code
  
 rrca                                            ;; [1]  Convert Pixel 1 format to Pixel 0, shifting bits to the right
 ld (cpct_spriteColourizeM1_px0_oldval), a       ;; [4]  Write Pixel 0 format (xxx0 xxx1) into Colourize function code
 ld (cpct_spriteMaskedColourizeM1_px0_oldval), a ;; [4]  Write Pixel 0 format (xxx0 xxx1) into Masked Colourize function code

ret         ;; [3] Return to the caller