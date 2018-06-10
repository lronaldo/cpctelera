;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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
.module cpct_strings

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setDrawCharM0
;;
;;    Sets foreground and background colours that will be used by <cpct_drawCharM0_inner>
;; when called. <cpct_drawCharM0_inner> is used by both <cpct_drawCharM0> and <cpct_drawStringM0>.
;;
;; C Definition:
;;    void <cpct_setDrawCharM0> (<u8> *fg_pen*, <u8> *bg_pen*) __z88dk_callee
;;
;; Input Parameters (2 Bytes):
;;  (1B L )  fg_pen       - Foreground palette colour index (Similar to BASIC's PEN, 0-15)
;;  (1B H )  bg_pen       - Background palette colour index (PEN, 0-15)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setDrawCharM0_asm
;;
;; Parameter Restrictions:
;;  * *fg_pen* must be in the range [0-15]. It is used to access a colour mask table and,
;; so, a value greater than 15 will return a random colour mask giving unpredictable 
;; results (typically bad character rendering, with odd colour bars).
;;  * *bg_pen* must be in the range [0-15], with identical reasons to *fg_pen*.
;;
;; Requirements and limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function sets the internal values of <cpct_drawCharM0_inner>, so that next calls
;; to <cpct_drawCharM0> or <cpct_drawStringM0> will use these new values. Concretely, these
;; values are *fg_pen* (foreground colour) and *bg_pen* (background colour). This function
;; receives 2 colour values in the range [0-15] and transforms them into the 4 possible 
;; combinations of these 2 colours in groups of 2 pixels. Namely, 
;; (start code)
;;                       ___________________
;;  2-bit combinations  | 00 | 01 | 10 | 11 |   * b = pixel coloured using bg_pen (background)
;;  Displayed pixels    | bb | bF | Fb | FF |   * F = pixel coloured using fg_pen (foreground)
;;                      \-------------------/
;; (end code)
;;    In mode 0, each of these 4 possible combinations corresponds to 1 byte in screen pixel 
;; format that encodes the 2 selected colours.
;;
;;    These 4 combinations are stored into an internal array called *dc_2pxtableM0*, which is
;; used by <cpct_drawCharM0_inner> to draw characters on the screen in mode 0, 2-pixels at a time.
;; This array remains constant unless a new call to <cpct_setDrawCharM0> changes it. This lets
;; the user change colours once and use them many times for subsequent calls to draw functions.
;;
;;    The appropriate use of this function is to call it each time a new pair of colours is
;; required for following character drawings to be made either with <cpct_drawCharM0> or with
;; <cpct_drawStringM0>. If the same colours are to be used for many drawings, a single call to 
;; <cpct_setDrawCharM0> followed by many drawing calls would be the best practice. 
;;
;;    You may found use examples consulting documentation for <cpct_drawCharM0> and <cpct_drawStringM0>.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    ASM bindings  - 37 bytes (+100 cpct_drawCharM0_inner = 137 bytes)
;;      C bindings  - 35 bytes (+100 cpct_drawCharM0_inner = 135 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; -------------------------------------------
;;   Any      |      59        |     236
;; -------------------------------------------
;; Asm saving |      -9        |     -36
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Global symbols
;;
.globl dc_mode0_ct   ;; Colour conversion table (16 values to convert PEN values [0-15] to their screen pixel format)
.globl dc_2pxtableM0 ;; Screen Pixel Format conversion table (4 pairs of pixels BG-BG, BG-FG, FG-BG, FG-FG)

   ;; Get Colour values converted to Screen Pixel Format
   ;;
   ld    de, #dc_mode0_ct  ;; [3]  DE = Pointer to the start of the Conversion Colour Table
  
   ;; Convert foreground colour and store it into B
   ld     b, h             ;; [1] Save Background Pen into C for later use
   ld     h, #0            ;; [2] | HL = L    (Index of the colour into the table [0-15])
   add   hl, de            ;; [3] HL = DE + L ([Table base + Index], Points to the converted value)
   ld     c, (hl)          ;; [2] C = Conversion of foreground colour to screen pixel format
  
   ;; Convert background colour and store it into C
   ld     h, #0            ;; [2] |
   ld     l, b             ;; [1] | HL = B    (Index of the colour into the table [0-15])
   add   hl, de            ;; [3] HL = DE + B ([Table base + Index], Points to the converted value)
   ld     b, (hl)          ;; [2] B = Conversion of background colour to screen pixel format

   ;; Produce the 4 combinations for pairs of pixels (BG-BG, BG-FG, FG-BG, FG-FG)
   ;; and store them in the placeholders inside <cpct_drawCharM0>
   ld    hl, #dc_2pxtableM0;; [3] HL Points to the 4-byte colour conversion table inside cpct_drawCharM0

   ;; Calculate and store BG-BG mode 0 pixel pair
   ld     a, b             ;; [1] A = Right Pixel is BG coloured
   rlca                    ;; [1] Shift left so A = Left pixel is BG coloured
   or     b                ;; [1] A = Left and Right pixels are BG coloured
   ld    (hl), a           ;; [2] Store BG-BG pair of mode 0 pixels in the table
   
   ;; Calculate and store BG-FG mode 0 pixel pair
   inc   hl                ;; [2] Point to next pair (BG-FG) in the table
   xor    b                ;; [1] Set Right Pixel to 0 (Its value is equal to B)
   or     c                ;; [1] Left Pixel: BG, Right Pixel: FG
   ld    (hl), a           ;; [2] Store BG-FG pair of mode 0 pixels in the table

   ;; Calculate and store FG-BG mode 0 pixel pair
   inc   hl                ;; [2] Point to next pair (FG-BG) in the table
   rlca                    ;; [1] Left Pixel: FG, Right Pixel: xx
   and   #0xAA             ;; [2] Right Pixel: 00
   or    b                 ;; [1] Left Pixel: FG, Right Pixel: BG
   ld    (hl), a           ;; [2] Store FG-BG pair of mode 0 pixels in the table

   ;; Calculate and store FG-FG mode 0 pixel pair
   inc   hl                ;; [2] Point to next pair (FG-BG) in the table
   xor    b                ;; [1] Left Pixel: FG, Right Pixel: 0
   or     c                ;; [1] Left Pixel: FG, Right Pixel: FG
   ld    (hl), a           ;; [2] Store FG-FG pair of mode 0 pixels in the table

   ;; Everything set up. Now return
   ret                     ;; [3] Return
