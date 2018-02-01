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
;; Function: cpct_spriteColourizeM1
;;
;;    Replace one concrete colour of a sprite by a different one. This function
;; does the replacement, use <cpct_setSpriteColourizeM1> to pick up colours.
;;
;; C Definition:
;;    void <cpct_spriteColourizeM1> (<u8> *width*, <u8> *height*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B HL) sprite - Source Sprite Pointer (array of pixel data)
;;  (1B C ) height - Sprite Height in bytes (>0)
;;  (1B B ) width  - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteColourizeM1_f_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be a pointer to the start of an array containing sprite's pixels data 
;; in screen pixel format. Sprite must be rectangular and all bytes in the array must be 
;; consecutive pixels, starting from top-left corner and going left-to-right, top-to-bottom 
;; down to the bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;;  * *width* must be the width of the sprite *in bytes*. Always remember that the width must be 
;; expressed in bytes and *not* in pixels.
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;
;; Details:
;;  <TODO>
;;
;; Known limitations:
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 38 bytes
;;   ASM-bindings - 35 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;; Best  Case |   21 + (5 + 27W)H      |   84 + (20 + 108W)H
;; Worst Case |   21 + (5 + 29W)H      |   84 + (20 + 116W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |      965 / 1029        |       3860 /  4116
;;  W=4,H=32  |     3637 / 3893        |      14548 / 15572
;; ----------------------------------------------------------------
;; Asm saving |         -12            |          -48
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes
;;
;; Credits:
;;    Original routine optimized by @Docent and discussed in CPCWiki :
;; http://www.cpcwiki.eu/forum/programming/cpctelera-colorize-sprite/
;;
;; Thanks to all of them for their help and support.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Save width value to restore it after each line 
ld     a, b          ;; [1] A = width
ld    (w_restore), a ;; [4] Save width into its restore place

;; As E register is free, use it as a cache for Pixel 3 Pattern of the colour to be replaced
px3_oldval = .+1
ld     e, #00        ;; [2] E = Pixel 3 2-bit Pattern of the colour to be replaced (0xxx1xxx)

;; Loop through all the bytes of the sprite, replacing colours that have the same
;; 2-bit pattern of the colour we want to replace. 
loop:
   ;; Check and replace Pixel 3
   ld     a, (hl)    ;; [2] A = Byte with 4 Mode 1 Pixels to be replaced
   and   #0b10001000 ;; [2] A ^= 0x88. Left out only the 2 bits of the Pixel 0 (0xxx1xxx)
   ld     d, a       ;; [1] D = holds a copy of Pixel 3 bits, just in case we don't have to replace it
   cp     e          ;; [1] A == E? Check if A is equal to the Pixel 3 Pattern of the colour we want to replace
   jr    nz, notpx3  ;; [2/3] If it is not equal, just continue to check pixel 0
      px3_newval = .+1
      ld     d, #00  ;; [2] Perform replacement of Pixel 3. D holds the 2 bits of the new colour. #00 is a placeholder
   notpx3:

   ;; Check and replace Pixel 2
   ld     a, (hl)    ;; [2] A = Restore the value of the byte with the 4 Mode 1 pixels to be replaced
   and   #0b01000100 ;; [2] A ^= 0x44. Left out only the 2 bits of the Pixel 0 (0xxx1xxx)
   px2_oldval = .+1
   cp    #00         ;; [2] Check if A is equal to the Pixel 0 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx2  ;; [2/3] If it is not equal, just continue to mix both output values
      px2_newval = .+1
      ld     a, #00  ;; [2] Perform replacement of Pixel 0. A holds the 4 bits of the new colour. #00 is a placeholder
   notpx2:
	  
   ;; Mix both replacements
   or     d          ;; [1] A |= D. A and D hold replacements for pixels 0 and 1. This mixes them into A.
   ld  d, a          ;; [1] D = A. Save current replacements
   
   ;; Check and replace Pixel 1
   ld     a, (hl)    ;; [2] A = Restore the value of the byte with the 4 Mode 1 pixels to be replaced
   and   #0b00100010 ;; [2] A ^= 0x22. Left out only the 2 bits of the Pixel 0 (0xxx1xxx)
   px1_oldval = .+1
   cp    #00         ;; [2] Check if A is equal to the Pixel 0 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx1  ;; [2/3] If it is not equal, just continue to mix both output values
      px1_newval = .+1
      ld     a, #00  ;; [2] Perform replacement of Pixel 0. A holds the 4 bits of the new colour. #00 is a placeholder
   notpx1:
   
   ;; Mix with previous replacements
   or     d          ;; [1] A |= D. A and D hold replacements for pixels 0, 1 and 2. This mixes them into A.
   ld  d, a          ;; [1] D = A. Save current replacements
   
    ;; Check and replace Pixel 0
   ld     a, (hl)    ;; [2] A = Restore the value of the byte with the 4 Mode 1 pixels to be replaced
   and   #0b00010001 ;; [2] A ^= 0x11. Left out only the 2 bits of the Pixel 0 (0xxx1xxx)
   px0_oldval = .+1
   cp    #00         ;; [2] Check if A is equal to the Pixel 0 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx0  ;; [2/3] If it is not equal, just continue to mix both output values
      px0_newval = .+1
      ld     a, #00  ;; [2] Perform replacement of Pixel 0. A holds the 4 bits of the new colour. #00 is a placeholder
   notpx0:
   
   ;; Mix  with previous replacements and save
   or     d          ;; [1] A |= D. A and D hold replacements for pixels 0, 1, 2, 3 and 4. This mixes them into A.
   ld   (hl), a      ;; [2] Write byte with colours replaced
   inc   hl          ;; [2] HL++ Move to next byte of the sprite

 djnz  loop          ;; [3/4] B--. Continue looping if there are more bytes left in this sprite line (B!=0)

   w_restore=.+1
   ld    b, #00      ;; [2] B = width (restore value). #00 is a placeholder
   dec   c           ;; [1] C-- (One less line of the sprite to process)
 jr    nz, loop      ;; [2/3] Continue looping if there are more lines left

 ret                 ;; [3] Return to caller

;;
;; Global symbols to be used by external functions to set placeholders
;;
cpct_spriteColourizeM1_px0_oldval == px0_oldval
cpct_spriteColourizeM1_px1_oldval == px1_oldval
cpct_spriteColourizeM1_px2_oldval == px2_oldval
cpct_spriteColourizeM1_px3_oldval == px3_oldval

cpct_spriteColourizeM1_px0_newval == px0_newval
cpct_spriteColourizeM1_px1_newval == px1_newval
cpct_spriteColourizeM1_px2_newval == px2_newval
cpct_spriteColourizeM1_px3_newval == px3_newval
