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
;; Function: cpct_drawSpriteColorizeM0
;;
;;    Directly replace a color and draw a sprite to video memory.
;;
;; C Definition:
;;    void <cpct_drawSpriteColorizeM0> (void* *sprite*, void* *memory*, <u8> *width*, <u8> *height*, <u16> *rplcPat*) __z88dk_callee;
;;
;; Input Parameters (8 bytes):
;;  (2B HL) sprite      - Source Sprite Pointer (array of pixel data)
;;  (2B AF) memory      - Destination video memory pointer
;;  (1B C ) height      - Sprite Height in bytes (>0)
;;  (1B B ) width       - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (2B DE) rplcPat     - Replace Pattern => 1st byte(D)=Pattern to Find, 2nd byte(E)=Pattern to insert instead
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteColorizeM0_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *width* must be the width of the sprite *in bytes*. Always remember that the width must be 
;; expressed in bytes and *not* in pixels.
;;  The correspondence is mode 0 : 1 byte = 2 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *rplcPat* ([0000-FFFF], 16bits, unsigned) should contain 2 8-bit colour pixel 
;; patterns in Mode 1 screen pixel format (see below for an explanation of patterns).
;; Any value is valid, but values different of what you actually want will probably
;; result in estrange colours in the final array/sprite.
;;
;; Known limitations:
;;     * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;     * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * This function requires the CPC firmware to be DISABLED. Otherwise, random crashes might happen due to side effects.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL, IX, IY
;;
;; Required memory:
;;     C-bindings - 118 bytes
;;   ASM-bindings - 108 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;            |     29 + (62 + 23W)H   |    116 + (248 + 92W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |         1757           |        7028
;;  W=4,H=32  |         4957           |       19828
;; ----------------------------------------------------------------
;; Asm saving |         -16            |        -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;
;; Credits:
;;    Original routine optimized by @Docent and discussed in CPCWiki :
;; http://www.cpcwiki.eu/forum/programming/cpctelera-colorize-sprite/
;;
;; Thanks to all of them for their help and support.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   push  af        ;; [4] Save AF (Source Sprite Pointer) in Stack

   ;; Compute E = (FindPat ^ InsrPat). This will be used at the end of the routine
   ;; to insert InsrPat in the byte by XORing again, as the final operation will
   ;; be performed only on the bits of the SelectedPixels (those coinciding with
   ;; FindPat). This final operation will then be (1) XOR against FindPat (Zeroing bits,
   ;; because they are equal) and (2) XOR againts InsrPat (Inserting InsrPat bits, 
   ;; because its an XOR against zeros). That way, we will perform 2 operations on 1.
   ld    a, l      ;; [1] / IYL = (InsrPat ^ FindPat)
   xor   h         ;; [1] |
   ld__iyl_a       ;; [2] \ 

   ld    a, h      ;; [1] / IXL = H (FindPat) Save for later use
   ld__ixl_a       ;; [2] \

   ld__iyh_c       ;; [2] IYH = C (Sprite Width) Save for later use
   pop   hl        ;; [3] HL = Recover from Stack (Source Sprite Pointer)

   ;; Loop for all the bytes of the Sprite then Modify pixels of each byte using Patterns
height_loop:
   push  de        ;; [4] Save DE for later use (jump to next screen line)       

width_loop:
   push  bc        ;; [4] Save BC because need BC registers for computation
   ld    a ,(de)   ;; [2] Get background byte into A
   and  (hl)       ;; [2] Erase background part that is to be overwritten (Mask step 1)
   ld    b, a      ;; [1] B = A = Erased background part
   inc   hl        ;; [2] ++HL => Point HL to Sprite Colour information
   
   ;; First, perfom an XOR between the FindPat and the SpriteByte to modify
   ;; This XOR will convert to zero all bits of pixels coinciding with FindPat
   ;; But will left at least a bit with a 1 in the others (not coinciding with FindPat)
   ld__a_ixl       ;; [2] / *HL = SpriteByte, IXL = FindPat
   xor  (hl)       ;; [2] | C = A = (SpriteByte ^ FindPat)
   ld    c, a      ;; [1] \   => SelectedPixels=00, OtherPixels!=00 
   
   ;; Now we want to create a MASK byte with 0's in all the bits corresponding to
   ;; selected pixels (coinciding with FindPat) and 1's in all other bits. To do this,
   ;; as each pixels has 2 bits, we just need to OR both of them. Pixels selected have
   ;; both of them equal to 0, and non-selected have at least a 1. We rotate the
   ;; byte to align both pair of bits in each pixel, then we or them together.
   rrca            ;; [1] / E   = [  A  1  B  2  C  3  D  4 ]
   rrca            ;; [1] | A   = [  D  4  A  1  B  2  C  3 ]
   or    c         ;; [1] \ A|C = [ AD 14 AB 12 BC 23 CD 34 ]
   ld    c, a      ;; [1] C = A
   rrca            ;; [1] /   
   rrca            ;; [1] |  
   rrca            ;; [1] | D   = [   AD   14   AB   12   BC   23   CD   34 ]
   rrca            ;; [1] | A   = [   BC   23   CD   34   AD   14   AB   12 ]
   or    c         ;; [1] \ A|C = [ ABCD 1234 ABCD 1234 ABCD 1234 ABCD 1234 ]
   ;; We revert the MASK (producing ~MASK), making selected pixels be 11, and Others 00
   cpl             ;; [1] A = ~MASK (SelectedPixels==11, OtherPixels==00)

   ;; Now we are ready to insert InsrPath. First we multiply (AND) our negated mask (~MASK)
   ;; with E=(InsrPath ^ FindPat). This does the effect of Masquerading (InsrPat ^ FindPat), 
   ;; leaving 0's on all pixels that do not have to be modified, and leaving the others 
   ;; untouched. Those others will be XORed with the original SpriteByte, producing 2 
   ;; operations (SpriteByte ^ FindPat) ^ InsrPat, but only on the bits of SelectedPixels
   ;; (because of the previous Masquerading operation). Those 2 operations are (1) Zeroing
   ;; bits, (2) inserting InsrPat (because we XOR against zeros). Result is FindPat removed
   ;; and InsrPat inserted, but only on the bits of the SelectedPixels (as others are 0's 
   ;; after Masquerading).
   and__iyl        ;; [2] A = ~MASK & (InsrPat ^ FindPat)
   xor  (hl)       ;; [2] A = (~MASK & (InsrPat ^ FindPat)) ^ SpriteByte
   
   or    b         ;; [2] Add up background and sprite information in one byte (Mask step 2)
   ld   (de), a    ;; [2] Save modified background + sprite data information into memory
   inc   de        ;; [2] / Next bytes (sprite and memory)
   inc   hl        ;; [2] \
   
   pop   bc        ;; [3] Restore BC counters
   
   dec   c         ;; [1] --C holds sprite width, we decrease it to count pixels in this line.
   jp    nz, width_loop   ;; [2/3] While not 0, we are still painting this sprite line 
   
   pop   de        ;; [3] Restore DE start line (DestMem)   
   dec   b         ;; [1] --B holds sprite height. We decrease it to count another pixel line finished
   jp    z, end_sprite_colourize ;; [2/3] If 0, we have finished the last sprite line.
   
   ld__c_iyh       ;; [2] C = IYH (Sprite Width)
   ld    a, d      ;; [1] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08     ;; [2] so we add it to DE (just by adding 0x08 to D)
   ld    d, a      ;; [1]
   and   #0x38     ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jp    nz, height_loop  ;; [2/3] .. by checking the 4 bits that identify present memory line. 

   ld    a, e      ;; [1] If 0, we have crossed boundaries then DE = DE + 0xC050h
   add   #0x50     ;; [2] -- Relocate DE pointer to the start of the next pixel line:
   ld    e, a      ;; [1] -- DE is moved forward 3 memory banks plus 50 bytes (4000h * 3) 
   ld    a, d      ;; [1] -- which effectively is the same as moving it 1 bank backwards and then
   adc   #0xC0     ;; [2] -- 50 bytes forwards (which is what we want to move it to the next pixel line)
   ld    d, a      ;; [1] -- Calculations are made with 8 bit maths as it is faster than other alternatives here
   jp    height_loop      ;; [3] Jump to continue with next pixel line 

end_sprite_colourize:
                   ;; return in binding
