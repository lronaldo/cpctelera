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
;; Function: cpct_spriteMaskedColourizeM1
;;
;;    Replace one concrete colour of a masked sprite by a different one, ignoring 
;; interlaced mask bytes. This function does the replacement, use 
;; <cpct_setSpriteMaskedColourizeM1> to pick up colours.
;;
;; C Definition:
;;    void <cpct_spriteMaskedColourizeM1> (<u8> *width*, <u8> *height*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (2B HL) sprite - Source Sprite Pointer (array with pixel and mask data)
;;  (1B C ) height - Sprite Height in bytes
;;  (1B B ) width  - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteMaskedColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; along with mask data. Each mask data byte must precede its associated colour data byte.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be 
;; 2 x *width* x *height* (mask data doubles array size). 
;;  * *width* (1-256) must be the width of the sprite *in bytes* (not taking into account
;; mask bytes). Always remember that the width must be expressed in bytes and *not* in pixels.
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;  * *Beware!* A 0 value either for *width* or *height* will be treated as 256, and 
;; will probably lead this function to overwrite memory values outside your sprite array.
;;
;; Details:
;;    This function takes a masked *sprite* and replaces all pixels of a given colour value 
;; (*oldColour*) for a different one (*newColour*). Replacement ignores mask values: 
;; transparent pixels will continue to be invisible. Both colours had to be previously
;; selected by calling the function <cpct_setSpriteMaskedColourizeM1>. This function only
;; performs the replacement.
;;    Selected colours are inserted directly as immediate values into the code
;; of this function. After a call to <cpct_setSpriteMaskedColourizeM1>, machine code
;; that does the replacement gets modified permanently unless <cpct_setSpriteMaskedColourizeM1>
;; is called again. Therefore, you may perform one single call to <cpct_setSpriteMaskedColourizeM1>
;; to configure this function for many uses, resulting in a great performance gain.
;;    By default, this function would replace colour 0 by colour 0, producing no
;; effect at all. So, at least one call to <cpct_setSpriteMaskedColourizeM1> is required
;; before properly using this function.
;;
;; Example,
;; (start code)
;;  // Simple soccer ball Mode 1 8x8-pixels masked sprite
;;  const u8 g_ball[2*2*8] = {
;;       0xEE, 0x01, 0x77, 0x08,    0xCC, 0x30, 0x33, 0x0C
;;    ,  0x88, 0x43, 0x11, 0xC2,    0x00, 0xC3, 0x00, 0xC3
;;    ,  0x00, 0x3C, 0x00, 0x3C,    0x88, 0x34, 0x11, 0x2C
;;    ,  0xCC, 0x03, 0x33, 0xC0,    0xEE, 0x01, 0x77, 0x08
;;  }
;;  u8 g_ball_colours[2];
;;
;;  //
;;  // Soccer ball is made of patches of two colours. This
;;  // function changes colours of both patches.
;;  //
;;  void changeSocerBallColours(u8 newColour1, u8 newColour2) {
;;     // Change colour of the first patch of the ball
;;     cpct_setSpriteMaskedColourizeM1(g_ball_colours[0], newColour1);
;;     cpct_spriteMaskedColourizeM1(2, 8, g_ball);
;;     
;;     // Change colour of the second patch of the ball
;;     cpct_setSpriteMaskedColourizeM1(g_ball_colours[1], newColour2);
;;     cpct_spriteMaskedColourizeM1(2, 8, g_ball);
;;  
;;     // Save new ball colours for future changes
;;     g_ball_colours[0] = newColour1;
;;     g_ball_colours[1] = newColour2;
;;  }
;; (end code)
;;
;; Known limitations:
;;    * <cpct_setSpriteMaskedColourizeM1> should have been called at least once before
;; properly using this function. Otherwise, this function will produce no effect.
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;    * This function does not check for parameters being valid. Incorrect values
;; will probably produce changes in memory places outside your sprite, leading
;; to undefined behaviour.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 61 bytes
;;   ASM-bindings - 58 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;; Best  Case |   19 + (5 + 50W)H      |   76 + (20 + 200W)H
;; Worst Case |   19 + (5 + 53W)H      |   76 + (20 + 212W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |     1699 / 1795        |       6796 /  7180
;;  W=4,H=32  |     6579 / 6963        |      26316 / 27852
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
ld    a, b               ;; [1] A = width
ld    (w_restore), a     ;; [4] Save width into its restore place

;; Loop through all the bytes of the sprite, replacing colours that have the same
;; 2-bit pattern of the colour we want to replace. 
loop:
   inc    hl             ;; [2] HL++ Jump to the next byte (jump over mask byte)

   ;; Check and replace Pixel-3
   ld     e, (hl)        ;; [2] E = Byte with 4 Mode-1 Pixels to be replaced
   ld     a, #0b10001000 ;; [2] A = 0x88. Mask for the 2 bits of Pixel-3
   and    e              ;; [1] A = E ^ A. Left out only the 2 bits of Pixel-3 (1xxx0xxx)
   ld     d, a           ;; [1] D = holds a copy of Pixel-3 bits, just in case we don't have to replace it
   px3_oldval = .+1
   cp     #00            ;; [2] Check if A is equal to the Pixel-3 Pattern of the colour we want to replace
   jr    nz, notpx3      ;; [2/3] If it is not equal, just continue to check Pixel-2
      px3_newval = .+1
      ld     d, #00      ;; [2] Perform replacement of Pixel-3. D holds the 2 bits of the new colour. #00 is a placeholder
   notpx3:

   ;; Check and replace Pixel-2
   ld     a, #0b01000100 ;; [2] A = 0x44. Mask for the 2 bits of Pixel-2
   and    e              ;; [1] A = E ^ A. Left out only the 2 bits of Pixel-2 (x1xxx0xx)
   px2_oldval = .+1
   cp    #00             ;; [2] Check if A is equal to the Pixel-2 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx2      ;; [2/3] If it is not equal, just continue to mix both output values
      px2_newval = .+1
      ld     a, #00      ;; [2] Perform replacement of Pixel-2. A holds the 2 bits of the new colour. #00 is a placeholder
   notpx2:
	  
   ;; Mix Pixel-3 and Pixel-2 replacements
   or     d              ;; [1] A |= D. A and D hold replacements for pixels 3 and 2. This mixes them into A.
   ld     d, a           ;; [1] D = A. Save current replacements into D
   
   ;; Check and replace Pixel-1
   ld     a, #0b00100010 ;; [2] A = 0x22. Mask for the 2 bits of Pixel-1
   and    e              ;; [1] A = E ^ A. Left out only the 2 bits of Pixel-1 (xx1xxx0x)
   px1_oldval = .+1
   cp    #00             ;; [2] Check if A is equal to the Pixel-1 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx1      ;; [2/3] If it is not equal, just continue to mix both output values
      px1_newval = .+1
      ld     a, #00      ;; [2] Perform replacement of Pixel-1. A holds the 2 bits of the new colour. #00 is a placeholder
   notpx1:
   
   ;; Mix Pixel-1 with previous Pixel-3 & Pixel-2 already mixed replacements
   or     d              ;; [1] A |= D. D hold mixed replacements for pixels 3 and 2.
                         ;; ....A holds replacement for pixel-1. Both 3 replacements get mixed into A.
   ld     d, a           ;; [1] D = A. Save current replacements into D
   
    ;; Check and replace Pixel-0
   ld     a, #0b00010001 ;; [2] A = 0x11. Mask for the 2 bits of Pixel-0
   and    e              ;; [1] A = E ^ A. Left out only the 2 bits of Pixel-0 (xxx1xxx0)
   px0_oldval = .+1
   cp    #00             ;; [2] Check if A is equal to the Pixel-0 Pattern of the colour we want to replace. #00 is a placeholder
   jr    nz, notpx0      ;; [2/3] If it is not equal, just continue to mix both output values
      px0_newval = .+1
      ld     a, #00      ;; [2] Perform replacement of Pixel-0. A holds the 2 bits of the new colour. #00 is a placeholder
   notpx0:
   
   ;; Mix Pixel-0 with previous replacements and save
   or     d          ;; [1] A |= D. D hold mixed replacements for pixels 3, 2 and 1.
                     ;; ....A holds replacement for Pixel-0. Both 4 replacements get mixed into A.
   ld    (hl), a     ;; [2] Write byte with colours all 4 colours replaced
   inc    hl         ;; [2] HL++ Move to next byte of the sprite

djnz  loop           ;; [3/4] B--. Continue looping if there are more bytes left in this sprite line (B!=0)

   w_restore=.+1
   ld    b, #00      ;; [2] B = width (restore value). #00 is a placeholder
   dec   c           ;; [1] C-- (One less line of the sprite to process)
jr    nz, loop       ;; [2/3] Continue looping if there are more lines left

ret                  ;; [3] Return to caller

;;
;; Global symbols to be used by external functions to set placeholders
;;
cpct_spriteMaskedColourizeM1_px0_oldval == px0_oldval
cpct_spriteMaskedColourizeM1_px1_oldval == px1_oldval
cpct_spriteMaskedColourizeM1_px2_oldval == px2_oldval
cpct_spriteMaskedColourizeM1_px3_oldval == px3_oldval

cpct_spriteMaskedColourizeM1_px0_newval == px0_newval
cpct_spriteMaskedColourizeM1_px1_newval == px1_newval
cpct_spriteMaskedColourizeM1_px2_newval == px2_newval
cpct_spriteMaskedColourizeM1_px3_newval == px3_newval
