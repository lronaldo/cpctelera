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
;; Function: cpct_spriteColorizeM1
;;
;;    Replace a color in a sprite and copy to another or the same sprite.
;;
;; C Definition:
;;    void <cpct_spriteColorizeM1> (void* *sprite*, void* *spriteColor*, <u8> *width*, <u8> *height*, <u8> *oldColor*, <u8> *newColor*) __z88dk_callee;
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
;;    > call cpct_spriteColorizeM1_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;;  * *spriteColor*  must be an array containing the new sprite's pixels data with the new color.
;;  * *width* must be the width of the sprite *in bytes*. Always remember that the width must be 
;; expressed in bytes and *not* in pixels.
;;  The correspondence is mode 1 : 1 byte = 4 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *oldColor* must be the index of color (0 to 3) to replace
;;  * *newColor* must be the index of the new color (0 to 3)
;;
;; Known limitations:
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * This function requires the CPC firmware to be DISABLED. Otherwise, random crashes might happen due to side effects.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL, BC', DE', HL', IX
;;
;; Required memory:
;;     C-bindings - 73 bytes
;;   ASM-bindings - 60 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;            |    29 + (6 + 61W)H     |    116 + (24 + 244W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |        2077            |          8308
;;  W=4,H=32  |        8029            |         32116
;; ----------------------------------------------------------------
;; Asm saving |         -16            |          -64
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
.globl dc_mode1_ct

;; Macro to convert color to pixel Mode1 : Axxx Axxx
.macro convertPixel        ;; From cpct_px2byteM1                   
    ld   bc, #dc_mode1_ct  ;; [3] BC points to conversion table (dc_mode1_ct)
    
    ;; Compute HL += A
    add  c                 ;; [1] | C += A
    ld   c, a              ;; [1] |
    sub  a                 ;; [1] A = 0 (preserving Carry Flag)
    adc  b                 ;; [1] | L += Carry
    ld   b, a              ;; [1] |

    ;; A = *(BC + A)
    ld   a, (bc)           ;; [2] A = Value stored at the table pointed by BC 
.endm
          
    ;; Convert newColor to pixel format (E)
    ld a, h                ;; [1]  A = H new color index
    convertPixel           ;; [10] | Convert into A
    ld e, a                ;; [1]  | E = A new color : Axxx Axxx

    ;; Convert oldColor to pixel format (D)
    ld a, l                ;; [1]  A = L old color index
    convertPixel           ;; [10] | Convert into A
    ld d, a                ;; [1]  | D = A old color : Axxx Axxx
    
    ld c, #0x88            ;; [2] C = First Mask to get pixel A  : Axxx Axxx
    
    exx                    ;; [1] Switch to Alternate registers
    
    ld__ixl_c              ;; [1] IXL = C (Width)
    ld c, b                ;; [1] C = B (Height)
    
convertLoop:    
    ld__b_ixl              ;; [2] B = IXL (Sprite Width)
        
lineLoop:
    ld  a, (hl)            ;; [2] A = (HL) current Byte of sprite
    exx                    ;; [1] Switch to Default registers
    
    ld  l, a               ;; [1] L = A current Byte of sprite : ABCD ABCD
    and c                  ;; [2] A |= C (C = 0x88)            : Axxx Axxx
    
    cp  d                  ;; [1] Test if pixel (A) is the old colour to be replaced (D)
    jr  nz, readPixelA     ;; [2/3] If not equal go to next pixel 
        ld  a, e           ;; [1] else A = new colour to set (E)
    
readPixelA:
    ld  h, a               ;; [1] H = A (current colorized sprite) : Axxx Axxx
    
    sla l                  ;; [2] L (current byte of sprite) << 1 : ABCD ABCD -> BCDx BCDx
    ld  a, l               ;; [1] A = L  : BCDx BCDx
    and c                  ;; [2] A |= Mask (0x88) : Bxxx Bxxx
    
    cp  d                  ;; [1] Test if pixel (A) is the old colour to be replaced (D)
    jr  nz, readPixelB     ;; [2/3] If not equal go to next pixel 
        ld  a, e           ;; [1] else A = new colour to set (E)
        
readPixelB:
    rrca                   ;; [1] A = Axxx Axxx >> 1  : xBxx xBxx
    or h                   ;; [1] A |= H (color byte) : Axxx Axxx
    ld  h, a               ;; [1] H = A               : ABxx ABxx
    
    sla l                  ;; [2] L ( BCDx BCDx) << 1 : CDxx CDxx  
    ld  a, l               ;; [1] A = L               : CDxx CDxx  
    and c                  ;; [1] A |= C (C = 0x88)   : Cxxx Cxxx
    
    cp  d                  ;; [1] Test if pixel (A) is the old colour to be replaced (D)
    jr  nz, readPixelC     ;; [2/3] If not equal go to next pixel 
        ld  a, e           ;; [1] else A = new colour to set (E)
    
readPixelC:
    rrca                   ;; [1] A = Axxx Axxx >> 1  : xBxx xBxx
    rrca                   ;; [1] A = xxCx xxCx << 1  : xxCx xxCx
    or  h                  ;; [1] A |= H (ABxx ABxx)  : ABCx ABCx
    ld  h, a               ;; [1] H = A               : ABCx ABCx

    sla l                  ;; [2] L (BCDx BCDx) << 1  : Dxxx Dxxx  
    ld  a, l               ;; [1] A = L               : Dxxx Dxxx   
    and c                  ;; [1] A |= C (C = 0x88)   : Dxxx Dxxx
    
    cp  d                  ;; [1] Test if pixel (A) is the old colour to be replaced (D)
    jr  nz, readPixelD     ;; [2/3] If not equal go to next pixel 
        ld  a, e           ;; [1] else A = new colour to set (E)
    
readPixelD:    
    rrca                   ;; [1] A = Axxx Axxx >> 1  : xBxx xBxx
    rrca                   ;; [1] A = xBxx xBxx >> 1  : xxCx xxCx
    rrca                   ;; [1] A = xxCx xxxx >> 1  : xxxD xxxD
    or  h                  ;; [1] A |= H (ABCx ABCx)  : ABCD ABCD
  
setByte:  
    exx                    ;; [1] Switch to Alternate registers

    ld  (de), a            ;; [2] Update current sprite byte with pixels
    inc  hl                ;; [2] Next byte sprite source
    inc  de                ;; [2] Next byte sprite colorized
    djnz lineLoop          ;; [3] Decrement B (Width) if != 0 goto lineLoop
    
nextLine:    
    dec    c                  ;; [1] Decrement C (Height) 
    jr     nz, convertLoop    ;; [2/3] If != O goto convertLoop
    
end:
    ;; Return is included in bindings
    
