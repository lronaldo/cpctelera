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
;; Function: cpct_spriteColorizeM0
;;
;;    Replace a color in a sprite and copy to another or the same sprite.
;;
;; C Definition:
;;    void <cpct_spriteColorizeM0> (void* *sprite*, void* *spriteColor*, <u8> *width*, <u8> *height*, <u8> *oldColor*, <u8> *newColor*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B HL') sprite      - Source Sprite Pointer (array of pixel data)
;;  (2B DE') spriteColor - Destination Sprite Pointer (can be also the Source Sprite) (array of pixel data)
;;  (1B C' ) height      - Sprite Height in bytes (>0)
;;  (1B B' ) width       - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (1B L )  oldColor    - Color to replace
;;  (1B H )  newColor    - New color
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteColorizeM0_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;;  * *spriteColor*  must be an array containing the new sprite's pixels data with the new color.
;;  * *width* must be the width of the sprite *in bytes*. Always remember that the width must be 
;; expressed in bytes and *not* in pixels.
;;  The correspondence is mode 0 : 1 byte = 2 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *oldColor* must be the index of color (0 to 15) to replace
;;  * *newColor* must be the index of the new color (0 to 15)
;;
;; Known limitations:
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * This function requires the CPC firmware to be DISABLED. Otherwise, random crashes might happen due to side effects.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL, BC', DE', HL', IX
;;
;; Required memory:
;;     C-bindings - 81 bytes
;;   ASM-bindings - 67 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;            |   29 + (6 + 33W)H      |  116 + (24 + 132W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |        1181            |        4724 
;;  W=4,H=32  |        4445            |       17780
;; ----------------------------------------------------------------
;; Asm saving |         -16            |        -64
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

.globl dc_mode0_ct

;; Macro to convert Pixel to xAxC xBxD format
.macro convertPixel              
    ;; From cpct_px2byteM0
    ld   bc, #dc_mode0_ct  ;; [3] BC points to conversion table (dc_mode0_ct)
    
    ;; Compute BC += A
    add  c                 ;; [1] | C += A
    ld   c, a              ;; [1] |
    sub  a                 ;; [1] A = 0 (preserving Carry Flag)
    adc  b                 ;; [1] | B += Carry
    ld   b, a              ;; [1] |

    ;; A = *(BC + A)
    ld   a, (bc)           ;; [2] A = Value stored at the table pointed by BC
.endm

    ;; Convert newColor to pixel format (E)
    ld a, h                ;; [1]  A = H new color index
    convertPixel           ;; [10] | Convert into A
    ld e, a                ;; [1]  | E = A new color      : xAxC xBxD

    ;; Convert oldColor to pixel format (D)
    ld a, l                ;; [1]  A = L old color index
    convertPixel           ;; [10] | Convert into A
    ld d, a                ;; [1]  | D = A old color      : xAxC xBxD
    
    ld c, #0x55            ;; [2] C = Mask to get pixel A : xAxC xBxD
    exx                    ;; [1] Switch to Alternate registers
    
    ld__ixl_c              ;; [1] IXL = C (Width)
    ld c, b                ;; [1] C = B (Height)
    
convertLoop:    
    ld__b_ixl              ;; [2] B = IXL (Sprite Width)
    
lineLoop:
    ld  a, (hl)            ;; [2] A = (HL) current Byte of sprite
    exx                    ;; [1] Switch to Default registers
    
    ld  l, a               ;; [1] L = A current Byte of sprite : ABAB ABAB
    and c                  ;; [2] A |= C (C = 0x55)            : xBxB xBxB
    
    cp  d                  ;; [1] Test if pixel (A) is the old colour to be replaced (D)
    jr  nz, readPixelA     ;; [2/3] If not equal go to next pixel 
        ld  a, e           ;; [1] else A = new colour to set (E)

;; Pixel Mode 0 = ABAB ABAB
readPixelA:
    ld  h, a               ;; [1] H = A (current colorized sprite) : xBxB xBxB
    ld  a, l               ;; [1] L = A current Byte of sprite     : ABAB ABAB
    rrca                   ;; [1] A (current byte of sprite) >> 1  : ABAB ABAB -> xABA BABA
    and c                  ;; [2] A |= Mask (0x55)                 : xAxA xAxA
    
    cp  d                  ;; [1] Test if pixel (A) is the old colour to be replaced (D)
    jr  nz, readPixelB     ;; [2/3] If not equal go to next pixel 
        ld  a, e           ;; [1] else A = new colour to set (E)
    
readPixelB:
    rlca                   ;; [1] A = xAxA xAxA << 1  : AxAx AxAx
    or  h                  ;; [1] A |= H (xBxB xBxB)  : ABAB ABAB
  
setByte:  
    exx                    ;; [1] Switch to Alternate registers

    ld  (de), a            ;; [2] Update current sprite byte with pixels
    inc  hl                ;; [2] Next byte sprite source
    inc  de                ;; [2] Next byte sprite colorized
    djnz lineLoop          ;; [3] Decrement B (Width) if != 0 goto lineLoop
    
nextLine:    
    dec c                  ;; [1] Decrement C (Height) 
    jr  nz, convertLoop    ;; [2/3] If != O goto convertLoop
    
end:
    ;; Return is included in bindings
    
