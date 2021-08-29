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
;;    void <cpct_spriteColourizeM1> (<u16> *rplcPat*, <u16> *size*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (2B DE) rplcPat- Pixel patterns for finding and replacing (see details)
;;  (2B BC) size   - Size of the sprite/array in bytes (width*height)
;;  (2B HL) sprite - Source Sprite Pointer (array of pixel data)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be a pointer to the start of an array containing sprite's pixels data 
;; in screen pixel format. Sprite must be rectangular and all bytes in the array must be 
;; consecutive pixels, starting from top-left corner and going left-to-right, top-to-bottom 
;; down to the bottom-right corner. Total amount of bytes in pixel array should be 
;; *width* x *height*.
;;  * *width* (1-256) must be the width of the sprite *in bytes*. Always remember that 
;; the width must be expressed in bytes and *not* in pixels.
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;  * *Beware!* A 0 value either for *width* or *height* will be treated as 256, and 
;; will probably lead this function to overwrite memory values outside your sprite array.
;;
;; Details:
;;    This function takes a *sprite* and replaces all pixels of a given colour value 
;; (*oldColour*) for a different one (*newColour*). Replacement ignores mask values: 
;; transparent pixels will continue to be invisible. Both colours had to be previously
;; selected by calling the function <cpct_setSpriteColourizeM1>. This function only
;; performs the replacement.
;;    Selected colours are inserted directly as immediate values into the code
;; of this function. After a call to <cpct_setSpriteColourizeM1>, machine code
;; that does the replacement gets modified permanently unless <cpct_setSpriteColourizeM1>
;; is called again. Therefore, you may perform one single call to <cpct_setSpriteColourizeM1>
;; to configure this function for many uses, resulting in a great performance gain.
;;    By default, this function would replace colour 0 by colour 0, producing no
;; effect at all. So, at least one call to <cpct_setSpriteColourizeM1> is required
;; before properly using this function.
;;
;; Example,
;; (start code)
;;  u8 g_alienSkinColour;
;;  
;;  void setAliensSkinColour(u8 colour) {
;;     // Set old skin colour to be replaced with new one
;;     cpct_setSpriteColourizeM1(g_alienSkinColour, colour);
;;     
;;     // Replace all skin colour with new one on all alien sprites
;;     cpct_spriteColourizeM1(G_ALIEN_A_W, G_ALIEN_A_H, g_alien_a_sprite);
;;     cpct_spriteColourizeM1(G_ALIEN_B_W, G_ALIEN_B_H, g_alien_b_sprite);
;;     cpct_spriteColourizeM1(G_ALIEN_M_W, G_ALIEN_M_H, g_alien_m_sprite);
;;  
;;     // Save new skin colour for later changes
;;     g_alienSkinColour = colour;
;;  }
;; (end code)
;;
;; Known limitations:
;;    * <cpct_setSpriteColourizeM1> should have been called at least once before
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
;;     C-bindings - 60 bytes
;;   ASM-bindings - 57 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;; Best  Case |   19 + (5 + 48W)H      |   76 + (20 + 192W)H
;; Worst Case |   19 + (5 + 51W)H      |   76 + (20 + 204W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |     1635 / 1731        |       6540 /  6924
;;  W=4,H=32  |     6323 / 6707        |      25292 / 26828
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
;; Thanks to all who participated in the discussion for their help and support.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



   
.include "macros/cpct_undocumentedOpcodes.h.s"
.macro ld__ixl_d
   .DW  #0x6ADD  ;; ld ixl, d
.endm

;; cpct_spriteColourizeM1_asm
;;    HL -> Sprite / Buffer
;;    BC -> Size
;;    DE -> Colour Patterns to Find and Replace
;;     D -> Find Pattern
;;     E -> Replace Pattern
;;
;; Modifies:
;;    AF, BC, DE, HL, IXl
;;
;; Time Measures
;;    us         = 7 + 22*S + 4*SS 
;;    us ( 2x 8) =   363
;;    us ( 4x 8) =   715
;;    us ( 2x16) =   715
;;    us ( 4x16) =  1419
;;    us ( 8x16) =  2827
;;    us ( 4x32) =  2827
;;    us ( 8x32) =  5647
;;    us (16x32) = 11283
;;    SS = 1 + [S / 256]
;;
cpct_spriteColourizeM1b_asm::

   ;; TODO: Wrap in a loop for all BC bytes
   ld     a, e    ;; [1] / 
   xor    d       ;; [1] | E = (InsrPat ^ FindPat)
   ld     e, a    ;; [1] \ 
   ld__ixl_d      ;; [2] IXL = D (FindPat) Save for later use

   inc    b       ;; [1] ++B  // B needs to be increased as it is decreased in the loop 
                  ;;          // condition before checked against 0. If 0, it turns to 255.
loop:
   ld__a_ixl      ;; [2] / 
   xor   (hl)     ;; [2] | A = D = (SpriteByte ^ FindPat)
   ld     d, a    ;; [1] \   => PixelsFound=00, OtherPixels!=00

   rrca           ;; [1] /
   rrca           ;; [1] | A = ROR(SpriteByte ^ FindPat, 4) | (SpriteByte ^ FindPat)
   rrca           ;; [1] |   => MASK (PixelsFound==00, OtherPixels==11)
   rrca           ;; [1] |
   or     d       ;; [1] \
   cpl            ;; [1] A = ~MASK (PixelsFound==11, OtherPixels==00)
   and    e       ;; [1] A = ~MASK & (InsrPat ^ FindPat)
   xor   (hl)     ;; [2] A = (~MASK & (InsrPat ^ FindPat)) ^ SpriteByte
   ld    (hl), a  ;; [2] Save modified byte
   ;; Move to next byte and check against the end
   inc   hl       ;; [2] ++HL
   ;; Dec "BC" and repeat while not 0
   dec    c       ;; [1] --C
   jr    nz, loop ;; [2/3] if (C > 0) then BC != 0, continue with the loop
   djnz  loop     ;; [3/4] if (--B > 0) then BC != 0, continue with the loop
   
   ret            ;; [3] Finished colourizing the sprite