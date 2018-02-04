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
;;    Sets colours to be used when <cpct_spriteColourizeM1> gets called. It sets
;; both colour to be replaced (*oldColour*) and replacement colour (*newColour*).
;;
;; C Definition:
;;    void <cpct_setSpriteColourizeM1> (<u8> *oldColour*, <u8> *newColour*) __z88dk_callee;
;;
;; Input Parameters (2 bytes):
;;  (1B L )  oldColour     - Colour to be replaced (Palette Index, 0-3)
;;  (1B H )  newColour     - New colour (Palette Index, 0-3)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setSpriteColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *oldColour* (0-3) must be the palette index of the colour to be replaced.
;;  * *newColour* (0-3) must be the palette index of the new colour.
;;
;; Details:
;;    This function sets the colours that <cpct_spriteColourizeM1> will use 
;; internally when called. It sets the colour to be searched (*oldColour*) and
;; the replacement colour (*newColour*). Once set, a call to <cpct_spriteColourizeM1> 
;; will transform each *oldColour* pixel into a *newColour* one. 
;;  	The function should be called at least once before using 
;; <cpct_spriteColourizeM1>. Otherwise, <cpct_spriteColourizeM1> would have
;; no effect at all (it would replace colour 0 by colour 0).
;; 	This function works by modifying <cpct_spriteColourizeM1>'s machine code, 
;; changing values of *oldColour* and *newColour*. Therefore, the change is 
;; permanent until a new change is performed. So you may call this function 
;; once and then call <cpct_spriteColourizeM1> many times using the same
;; colour configuration. You may see a code example by consulting 
;; <cpct_spriteColourizeM1>'s documentation.
;;
;; Known limitations:
;;    * This function works from ROM, but <cpct_spriteColourizeM1> must 
;; be in RAM, as it gets modified by this function.
;;    * This function does not check for parameters being valid. Values 
;; outside (0-3) will produce undefined behaviour, probably corrupting
;; colour values of any sprite modified with <cpct_spriteColourizeM1>.
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
.globl cpct_spriteColourizeM1_px3_newval
.globl cpct_spriteColourizeM1_px2_newval
.globl cpct_spriteColourizeM1_px1_newval
.globl cpct_spriteColourizeM1_px0_newval

.globl cpct_spriteColourizeM1_px3_oldval
.globl cpct_spriteColourizeM1_px2_oldval
.globl cpct_spriteColourizeM1_px1_oldval
.globl cpct_spriteColourizeM1_px0_oldval

;; Use Look-Up-Table to convert palette index colour to screen pixel format
;; This conversion is for the first pixel into the four pixels each byte 
;; has in mode 1 [3,2,1,0]. Therefore, only bits (0xxx 1xxx) will be produced.

;; Convert newColour to pixel format and
;; save it into cpct_setSpriteColourizeM1 machine code placeholders
ld a, h                                   ;; [1]  A = H new colour index
cpctm_lutget8 dc_mode1_ct, b, c           ;; [10] A = dc_mode0_ct[BC + A]. Get Pixel-3 replacement colour from Look-Up-Table
ld (cpct_spriteColourizeM1_px3_newval), a ;; [4]  Set replacement for Pixel-3 in cpct_setSpriteMaskedColourizeM1

rrca                                      ;; [1]  Convert replacement to Pixel-2 format (x0xx x1xx)
ld (cpct_spriteColourizeM1_px2_newval), a ;; [4]  Set replacement for Pixel-2 in cpct_setSpriteColourizeM1

rrca                                      ;; [1]  Convert replacement to Pixel-1 format (xx0x xx1x)
ld (cpct_spriteColourizeM1_px1_newval), a ;; [4]  Set replacement for Pixel-1 in cpct_setSpriteColourizeM1

rrca                                      ;; [1]  Convert replacement to Pixel-0 format (xxx0 xxx1)
ld (cpct_spriteColourizeM1_px0_newval), a ;; [4]  Set replacement for Pixel-0 in cpct_setSpriteColourizeM1

;; Convert oldColour to pixel format and
;; save it into cpct_setSpriteColourizeM1 machine code placeholders
ld a, l                                   ;; [1]  A = L old colour index
cpctm_lutget8 dc_mode1_ct, b, c           ;; [10] A = dc_mode0_ct[BC + A]. Get Pixel-3 searched colour from Look-Up-Table
ld (cpct_spriteColourizeM1_px3_oldval), a ;; [4]  Set searched for Pixel-3 in cpct_setSpriteMaskedColourizeM1

rrca                                      ;; [1]  Convert searched to Pixel-2 format (x0xx x1xx)
ld (cpct_spriteColourizeM1_px2_oldval), a ;; [4]  Set searched for Pixel-2 in cpct_setSpriteColourizeM1

rrca                                      ;; [1]  Convert searched to Pixel-1 format (xx0x xx1x)
ld (cpct_spriteColourizeM1_px1_oldval), a ;; [4]  Set searched for Pixel-1 in cpct_setSpriteColourizeM1

rrca                                      ;; [1]  Convert searched to Pixel-0 format (xxx0 xxx1)
ld (cpct_spriteColourizeM1_px0_oldval), a ;; [4]  Set searched for Pixel-0 in cpct_setSpriteColourizeM1

ret         ;; [3] Return to the caller