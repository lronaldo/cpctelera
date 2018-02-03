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
;; Function: cpct_setSpriteMaskedColourizeM1
;;
;;    Sets colours to be used when <cpct_spriteMaskedColourizeM1> gets called. It sets
;; both colour to be replaced (*oldColour*) and replacement colour (*newColour*).
;;
;; C Definition:
;;    void <cpct_setSpriteMaskedColourizeM1> (<u8> *oldColor*, <u8> *newColor*) __z88dk_callee;
;;
;; Input Parameters (2 bytes):
;;  (1B L )  oldColor     - Colour to be replaced (Palette Index, 0-4)
;;  (1B H )  newColor     - New colour (Palette Index, 0-4)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setSpriteMaskedColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *oldColor* must be the palette index of the colour to be replaced (0 to 4).
;;  * *newColor* must be the palette index of the new colour (0 to 4).
;;
;; Known limitations:
;;     * This function works from ROM, but <cpct_spriteMaskedColourizeM0> must 
;; be in RAM, as it gets modified by this function.
;;
;; Details:
;;    <TODO>
;;
;; Destroyed Register values: 
;;    A, BC, HL
;;
;; Required memory:
;;     C-bindings - 53 bytes
;;   ASM-bindings - 51 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;   Any      |          72            |        288
;; ----------------------------------------------------------------
;; Asm saving |          -9            |        -36
;; ----------------------------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl dc_mode1_ct                ;; Look-Up-Table to convert Palette Indexes to 2-bits pixel 0 screen format patterns
.include "macros/cpct_luts.h.s"   ;; Macros to easily access the Look-Up-Table

;; Symbols for placeholders inside the Colourize function
.globl cpct_spriteMaskedColourizeM1_px3_newval
.globl cpct_spriteMaskedColourizeM1_px2_newval
.globl cpct_spriteMaskedColourizeM1_px1_newval
.globl cpct_spriteMaskedColourizeM1_px0_newval

.globl cpct_spriteMaskedColourizeM1_px3_oldval
.globl cpct_spriteMaskedColourizeM1_px2_oldval
.globl cpct_spriteMaskedColourizeM1_px1_oldval
.globl cpct_spriteMaskedColourizeM1_px0_oldval

;; Local symbols that act as aliases for the global symbols.
;; This helps making code shorter and clearer
px3_newval = cpct_spriteMaskedColourizeM1_px3_newval
px2_newval = cpct_spriteMaskedColourizeM1_px2_newval
px1_newval = cpct_spriteMaskedColourizeM1_px1_newval
px0_newval = cpct_spriteMaskedColourizeM1_px0_newval
px3_oldval = cpct_spriteMaskedColourizeM1_px3_oldval
px2_oldval = cpct_spriteMaskedColourizeM1_px2_oldval
px1_oldval = cpct_spriteMaskedColourizeM1_px1_oldval
px0_oldval = cpct_spriteMaskedColourizeM1_px0_oldval

;; Use Look-Up-Table to convert palette index colour to screen pixel format
;; This conversion is for the first Pixel into the four pixels each byte 
;; has in mode 1 [3,2,1,0]. Therefore, only bits 0xxx1xxx will be produced.

;; Convert newColour to pixel format 
ld a, h                          ;; [1]  A = H new colour index
cpctm_lutget8 dc_mode1_ct, b, c  ;; [10] A = dc_mode0_ct[BC + A]. Get Pixel-3 replacement colour from Look-Up-Table
ld (px3_newval), a               ;; [4]  Set replacement for Pixel-3 in cpct_setSpriteMaskedColourizeM1

rrca                             ;; [1]  Convert replacement to Pixel-2 format (x0xx x1xx)
ld (px2_newval), a               ;; [4]  Set replacement for Pixel-2 in cpct_setSpriteMaskedColourizeM1

rrca                             ;; [1]  Convert replacement to Pixel-1 format (xx0x xx1x)
ld (px1_newval), a               ;; [4]  Set replacement for Pixel-1 in cpct_setSpriteMaskedColourizeM1

rrca                             ;; [1]  Convert replacement to Pixel-0 format (xxx0 xxx1)
ld (px1_newval), a               ;; [4]  Set replacement for Pixel-0 in cpct_setSpriteMaskedColourizeM1

;; Convert oldColour to pixel format 
ld a, h                          ;; [1]  A = H new colour index
cpctm_lutget8 dc_mode1_ct, b, c  ;; [10] A = dc_mode0_ct[BC + A]. Get Pixel-3 searched colour from Look-Up-Table
ld (px3_oldval), a               ;; [4]  Set searched for Pixel-3 in cpct_setSpriteMaskedColourizeM1

rrca                             ;; [1]  Convert searched to Pixel-2 format (x0xx x1xx)
ld (px2_oldval), a               ;; [4]  Set searched for Pixel-2 in cpct_setSpriteMaskedColourizeM1

rrca                             ;; [1]  Convert searched to Pixel-1 format (xx0x xx1x)
ld (px1_oldval), a               ;; [4]  Set searched for Pixel-1 in cpct_setSpriteMaskedColourizeM1

rrca                             ;; [1]  Convert searched to Pixel-0 format (xxx0 xxx1)
ld (px1_oldval), a               ;; [4]  Set searched for Pixel-0 in cpct_setSpriteMaskedColourizeM1

ret         ;; [3] Return to the caller