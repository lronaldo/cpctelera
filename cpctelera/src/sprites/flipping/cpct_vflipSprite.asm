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
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_vflipSprite
;;
;;   Vertically flips a sprite, encoded in screen pixel format, *mode 0*.
;;
;; C definition:
;;   void <cpct_vflipSprite> (<u8> width, <u8> height, void* spbl, void* sprite) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (1B  A) width  - Width of the sprite in *bytes* (*NOT* in pixels!). Must be >= 1.
;;  (1B  C) height - Height of the sprite in pixels / bytes (both are the same). Must be >= 1.
;;  (2B HL) spbl   - Pointer to the bottom-left byte of the sprite
;;  (2B DE) sprite - Pointer to the sprite array (top-left byte)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_vflipSprite_asm
;;
;; Parameter Restrictions:
;;  * *width*  must be the width of the sprite *in bytes* (Not! in pixels).
;;  * *height* must be the height of the sprite in pixels.
;;  * *spbl*   must be a pointer to the bottom-left byte of the *sprite*.
;;  * *sprite* must be a pointer to an array containing sprite's pixels data.
;;
;; Known limitations:
;;  * This function will not work from ROM, as it uses self-modifying code.
;;  * This function does not do any kind of boundary check. If you give it 
;; incorrect values for *width*, *height* or *sprite* pointer it might 
;; potentially alter the contents of memory locations beyond *sprite* boundaries. 
;; This could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that your values are correct.
;;
;; Details:
;;    <TODO>
;;
;; (start code)
;;    // Example call. Sprite has 8x4 pixels (4x4 bytes)
;;    cpct_hflipSpriteM0(4, 4, sprite);
;;
;;    // Operation performed by the call and results
;;    //
;;    // --------------------------------------------------------------
;;    //  |  Received as parameter     | Result after flipping       |
;;    // --------------------------------------------------------------
;;    //  | sprite => [05][21][73][40] |  sprite => [04][37][12][05] |
;;    //  |           [52][23][37][74] |            [47][73][32][25] |
;;    //  |           [05][11][31][04] |            [04][13][11][50] |
;;    //  |           [00][55][44][00] |            [00][44][55][00] |
;;    // --------------------------------------------------------------
;;    //  Sprite takes 16 consecutive bytes in memory (4 rows with 4 bytes)
;;    //
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - xx bytes
;;  ASM-bindings - xx bytes
;;
;; Time Measures: <TODO>
;; (start code)
;;  Case       |      microSecs (us)       |         CPU Cycles          |
;; -----------------------------------------------------------------------
;;  Even-width |     (32WW + 16)H + 32     |     (128WW +  64)H + 128    |
;;  Oven-width |     (32WW + 36)H + 37     |     (128WW + 144)H + 148    |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |           800             |           3200              |
;;  W=5,H=32   |          3237             |          12948              |
;; -----------------------------------------------------------------------
;;  Asm saving |          -12              |            -48              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *W* = *width* % 2, *WW* = *width*/2, *H* = *height*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.include "macros/cpct_maths.h.s"

   ld (wrestore), a  ;; [5]
   ld     b, a       ;; [1]
   srl    c          ;; [2]

loop:
      ld     a, (hl)    ;; [2]
      ex    af, af'     ;; [1]
      ld     a, (de)    ;; [2]
      ld  (hl), a       ;; [2]
      ex    af, af'     ;; [1]
      ld  (de), a       ;; [2]
      inc   hl          ;; [2]
      inc   de          ;; [2]

   djnz loop           ;; [3/4]

   dec   c              ;; [1]
   ret   z              ;; [2/4]

   ;; Next line
wrestore=.+1
   ld    a, #00         ;; [2] width
   ld    b, a           ;; [1]
   add   a              ;; [1]
   sub_de_a             ;; [7]

   jr loop              ;; [3]

