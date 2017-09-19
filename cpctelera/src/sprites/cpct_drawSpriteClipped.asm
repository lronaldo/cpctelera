;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) Arnaud BOUCHE
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawSpriteClipped
;;
;;    Copies partially a sprite from an array to video memory (or to a screen buffer).
;;
;; C Definition:
;;    void <cpct_drawSpriteClipped> (u8 width_to_draw, void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;  (1B A ) width_to_draw - Sprite Width to Draw in *bytes* (>0 && <= width) (Beware, *not* in pixels!)
;;  (2B HL) sprite - Source Sprite Pointer (array with pixel and mask data)
;;  (2B DE) memory - Destination video memory pointer
;;  (1B C ) width  - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B B ) height - Sprite Height in bytes (>0)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteClipped_asm
;;
;; Parameter Restrictions:
;;  * *width_to_draw* must be the width *in bytes* to draw partially the sprite  
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 
;; (<cpct_px2byteM1>) as for mode 2 is linear (1 bit = 1 pixel).
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *width* must be the width of the sprite *in bytes*, and must be in the range [1-63].
;; A sprite width outside the range [1-63] will probably make the program hang or crash, 
;; due to the optimization technique used. Always remember that the width must be 
;; expressed in bytes and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;
;; Known limitations:
;;    * See <cpct_drawSprite>
;;
;; Details:
;;    * See <cpct_drawSprite>
;;
;; Destroyed Register values: 
;;   AF', AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 47 bytes
;;  ASM-bindints - 42 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |    microSecs (us)        |    CPU Cycles
;; ----------------------------------------------------------------
;;  Best      |  20 + (33 + 11W)H        | 80 + (132 + 44W)H
;;  Worst     |       Best + 10          |     Best + 40
;; ----------------------------------------------------------------
;;  W=2,H=16  |          900             |       3600
;;  W=4,H=32  |         2484             |       9936
;; ----------------------------------------------------------------
;; Asm saving |          -16             |       -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   push ix                 ;; [5] Save IX register before using it as temporal var   
   
   ;; Compute Sprite offset
   ld__ixl_c               ;; [2] IXL = Sprite Width (C)
   ld  c, a				   ;; [1] C = Sprite Width To Draw (A)
   ld__a_ixl               ;; [2] A = Sprite Width (IXL)
   sub c                   ;; [1] A = Sprite Offset (Sprite Width (A) - Sprite Width to Draw (C))
   ld (offset_sprite), a   ;; [2] Set Sprite Offset at placeholder  
   ld__ixl_c               ;; [2] IXL = Sprite Width to Draw (C)
 
dms_sprite_height_loop:
   push de         ;; [4] Save DE for later use (jump to next screen line)

dms_sprite_width_loop:
   ld   a, (hl)    ;; [2] Get next background byte into A X 
   ld  (de), a     ;; [2] Save modified background + sprite data information into memory X
   inc  de         ;; [2] Next bytes (memory) X
   inc  hl         ;; [2] Next bytes (sprite) X

   dec   c         ;; [1] C holds sprite width, we decrease it to count pixels in this line.
   jr   nz,dms_sprite_width_loop;; [2/3] While not 0, we are still painting this sprite line 
                                ;;      - When 0, we have to jump to next pixel line

   pop  de         ;; [3] Recover DE from stack. We use it to calculate start of next pixel line on screen

   dec   b         ;; [1] B holds sprite height. We decrease it to count another pixel line finished
   jr    z,dms_sprite_copy_ended;; [2/3] If 0, we have finished the last sprite line.
                                ;;      - If not 0, we have to move pointers to the next pixel line									

   ;; Add Sprite Offset of current sprite data array position								
   offset_sprite = .+1       ;; Placeholder for the Sprite Offset
   ld  c, #00                ;; [2] C = Sprite Offset 0 is default value
   ld  a, b					 ;; [1] Save B into A
   ld  b, #00                ;; [1] Not need of B (Height) for computation
   add hl, bc                ;; [3] Add offset sprite color + mask
   ld  b, a                  ;; [1] Restore Height (B) from A
   ld__c_ixl                 ;; [2] Restore Sprite Width to Draw into C  

   ld    a, d      ;; [1] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08     ;; [2]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a      ;; [1]
   and   #0x38     ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr   nz, dms_sprite_height_loop ;; [2/3]  by checking the 4 bits that identify present memory line. 
                                   ;; ....  If 0, we have crossed boundaries

dms_sprite_8bit_boundary_crossed:
   ld    a, e      ;; [1] DE = DE + 0xC050h
   add   #0x50     ;; [2] -- Relocate DE pointer to the start of the next pixel line:
   ld    e, a      ;; [1] -- DE is moved forward 3 memory banks plus 50 bytes (4000h * 3) 
   ld    a, d      ;; [1] -- which effectively is the same as moving it 1 bank backwards and then
   adc   #0xC0     ;; [2] -- 50 bytes forwards (which is what we want to move it to the next pixel line)
   ld    d, a      ;; [1] -- Calculations are made with 8 bit maths as it is faster than other alternatives here

   jr  dms_sprite_height_loop ;; [3] Jump to continue with next pixel line

dms_sprite_copy_ended:
   pop  ix         ;; [5] Restore IX before returning
