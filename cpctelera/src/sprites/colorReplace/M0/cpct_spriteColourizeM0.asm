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
;; Function: cpct_spriteColourizeM0
;;
;;    Replace one concrete colour of a sprite by a different one. This function
;; does the replacement, use <cpct_setSpriteColourizeM0> to pick up colours.
;;
;; C Definition:
;;    void <cpct_spriteColourizeM0> (<u8> *width*, <u8> *height*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (2B HL) sprite - Source Sprite Pointer (array of pixel data)
;;  (1B C ) height - Sprite Height in bytes
;;  (1B B ) width  - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteColourizeM0_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be a pointer to the start of an array containing sprite's pixels data 
;; in screen pixel format. Sprite must be rectangular and all bytes in the array must be 
;; consecutive pixels, starting from top-left corner and going left-to-right, top-to-bottom 
;; down to the bottom-right corner. Total amount of bytes in pixel array 
;; should be *width* x *height*.
;;  * *width* (1-256) must be the width of the sprite *in bytes*. Always remember that 
;; the width must be expressed in bytes and *not* in pixels.
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;  * *Beware!* A 0 value either for *width* or *height* will be treated as 256, and 
;; will probably lead this function to overwrite memory values outside your sprite array.
;;
;; Details:
;;    This function modifies a *sprite* replacing pixels of a given colour by 
;; another colour. Both searched and replacement colour are previously 
;; selected using <cpct_setSpriteColourizeM0>. This function only performs 
;; the replacement of the previously selected colours.
;;    Selected colours are inserted directly as immediate values into the code
;; of this function. After a call to <cpct_setSpriteColourizeM0>, machine code
;; that does the replacement gets modified permanently unless <cpct_setSpriteColourizeM0>
;; is called again. Therefore, you may perform one single call to <cpct_setSpriteColourizeM0>
;; to configure this function for many uses, resulting in a great performance gain.
;;    If no call is performed to <cpct_setSpriteColourizeM0> before calling
;; this function, no color replacement will be done at all. Effectively, it will
;; replaced color 0 with color 0, to no effect at all.
;; Example,
;; (start code)
;; void SetEnemyTShirtsNewColour(u8 colour) {
;;    static u8 oldcolour;
;;    u8 i;
;;
;;    // Enemy t-shirts oldcolour will be replaced with new colour
;;    cpct_setSpriteColourizeM0(oldcolour, colour);
;;
;;    // Replace t-shirts colours from all enemy sprites. All enemy sprites
;;    // will be modified replacing oldcolour with colour.
;;    for(i=0; i < ENEMIES; ++i)
;;       cpct_spriteColourizeM0(G_ENEMY_W, G_ENEMY_H, gEnemy[i]->sprite);
;;
;;    // After replacement, colour becomes oldcolour
;;    oldcolour = colour;
;;  }
;; (end code)
;;
;; Known limitations:
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;    * <cpct_setSpriteColourizeM0> should have been called at least once before
;; properly using this function. Otherwise, this function will produce no effect.
;;    * This function does not check for parameters being valid. Incorrect values
;; will probably produce changes in memory places outside your sprite, leading
;; to undefined behaviour.
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

;; As E register is free, use it as a cache for Pixel 1 Pattern of the colour to be replaced
px1_oldval = .+1
ld     e, #00        ;; [2] E = Pixel 1 4-bit Pattern of the colour to be replaced (x0x2x1x3)

;; Loop through all the bytes of the sprite, replacing colours that have the same
;; 4-bit pattern of the colour we want to replace. 
loop:
   ;; Check and replace Pixel 1
   ld     a, (hl)    ;; [2] A = Byte with 2 Mode 0 Pixels to be replaced
   and   #0b01010101 ;; [2] A ^= 0x55. Left out only the 4 bits of the Pixel 1 (x0x2x1x3)
   ld     d, a       ;; [1] D = holds a copy of Pixel 1 bits, just in case we don't have to replace it
   cp     e          ;; [1] A == E? Check if A is equal to the Pixel 1 Pattern of the colour we want to replace
   jr    nz, notpx1  ;; [2/3] If it is not equal, just continue to check pixel 0
      px1_newval = .+1
      ld     d, #00  ;; [2] Perform replacement of Pixel 1. D holds the 4 bits of the new colour. #00 is a placeholder
   notpx1:

   ;; Check and replace Pixel 0
   ld     a, (hl)    ;; [2] A = Restore the value of the byte with the 2 Mode 0 pixels to be replaced
   and   #0b10101010 ;; [2] A ^= 0xAA. Left out only the 4 bits of the Pixel 0 (0x2x1x3x)
   px0_oldval = .+1
   cp    #00         ;; [2] Check if A is equal to the Pixel 0 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx0  ;; [2/3] If it is not equal, just continue to mix both output values
      px0_newval = .+1
      ld     a, #00  ;; [2] Perform replacement of Pixel 0. A holds the 4 bits of the new colour. #00 is a placeholder
   notpx0:

   ;; Mix both replacements and save
   or     d          ;; [1] A |= C. A and C hold replacements for pixels 0 and 1. This mixes them into A.
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
cpct_spriteColourizeM0_px0_oldval == px0_oldval
cpct_spriteColourizeM0_px1_oldval == px1_oldval
cpct_spriteColourizeM0_px0_newval == px0_newval
cpct_spriteColourizeM0_px1_newval == px1_newval
