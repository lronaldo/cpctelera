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
;; Function: cpct_setSpriteMaskedColourizeM0
;;
;;    Sets colours to be used when <cpct_setSpriteMaskedColourizeM0> gets called. It sets
;; both colour to be replaced (*oldColour*) and replacement colour (*newColour*).
;;
;; C Definition:
;;    void <cpct_setSpriteMaskedColourizeM0> (<u8> *oldColour*, <u8> *newColour*) __z88dk_callee;
;;
;; Input Parameters (2 bytes):
;;  (1B L )  oldColour    - Colour to be replaced (Palette Index, 0-15)
;;  (1B H )  newColour    - New colour (Palette Index, 0-15)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setSpriteMaskedColourizeM0_asm
;;
;; Parameter Restrictions:
;;  * *oldColour* (0-15) must be the palette index of the colour to be replaced.
;;  * *newColour* (0-15) must be the palette index of the new colour.
;;
;; Details:
;;    This function sets the colours that <cpct_setSpriteMaskedColourizeM0> will use 
;; internally when called. It sets the colour to be searched (*oldColour*) and
;; the replacement colour (*newColour*). Once set, a call to <cpct_setSpriteMaskedColourizeM0> 
;; will transform each *oldColour* pixel into a *newColour* one. 
;;  	The function should be called at least once before using 
;; <cpct_setSpriteMaskedColourizeM0>. Otherwise, <cpct_setSpriteMaskedColourizeM0> would have
;; no effect at all (it would replace colour 0 by colour 0).
;; 	This function works by modifying <cpct_setSpriteMaskedColourizeM0>'s machine code, 
;; changing values of *oldColour* and *newColour*. Therefore, the change is 
;; permanent until a new change is performed. So you may call this function 
;; once and then call <cpct_setSpriteMaskedColourizeM0> many times using the same
;; colour configuration. You may see a code example by consulting 
;; <cpct_setSpriteMaskedColourizeM0>'s documentation.
;;
;; Known limitations:
;;    * This function works from ROM, but <cpct_setSpriteMaskedColourizeM0> must 
;; be in RAM, as it gets modified by this function.
;;    * This function does not check for parameters being valid. Values 
;; outside (0-15) will produce undefined behaviour, probably corrupting
;; colour values of any sprite modified with <cpct_setSpriteMaskedColourizeM0>.
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
;;   Any      |          52            |        208
;; ----------------------------------------------------------------
;; Asm saving |          -9            |        -36
;; ----------------------------------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl dc_mode0_ct                ;; Look-Up-Table to convert Palette Indexes to 4-bits pixel 1 screen format patterns
.include "macros/cpct_luts.h.s"   ;; Macros to easily access the Look-Up-Table
;; Symbols for placeholders inside the Colourize function
.globl cpct_spriteMaskedColourizeM0_px1_newval
.globl cpct_spriteMaskedColourizeM0_px0_newval
.globl cpct_spriteMaskedColourizeM0_px1_oldval
.globl cpct_spriteMaskedColourizeM0_px0_oldval

 ;; Use Look-Up-Table to convert palette index colour to screen pixel format
 ;; This conversion is for the Pixel 1 into the two pixels each byte has in mode 0 [0,1]
 ;; Therefore, only bits x0x2x1x3 will be produced.

 ;; Convert newColour to pixel format 
 ld a, h                                            ;; [1]  A = H new colour index
 cpctm_lutget8 dc_mode0_ct, b, c                    ;; [10] Get from Look-Up-Table dc_mode0_ct[BC + A]
 ld (cpct_spriteMaskedColourizeM0_px1_newval), a    ;; [4]  Write Pixel 1 format (x0x2x1x3) into Colourize function code
 rlca                                               ;; [1]  Convert Pixel 1 format to Pixel 0, shifting bits to the left
 ld (cpct_spriteMaskedColourizeM0_px0_newval), a    ;; [4]  Write Pixel 0 format (0x2x1x3x) into Colourize function code

 ;; Convert oldColour to pixel format 
 ld a, l                                            ;; [1]  A = L old colour index
 cpctm_lutget8 dc_mode0_ct, b, c                    ;; [10] Get from Look-Up-Table dc_mode0_ct[BC + A]
 ld (cpct_spriteMaskedColourizeM0_px1_oldval), a    ;; [4]  Write Pixel 1 format (x0x2x1x3) into Colourize function code
 rlca                                               ;; [1]  Convert Pixel 1 format to Pixel 0, shifting bits to the left
 ld (cpct_spriteMaskedColourizeM0_px0_oldval), a    ;; [4]  Write Pixel 0 format (0x2x1x3x) into Colourize function code

ret                                                 ;; [3] Return to the caller
