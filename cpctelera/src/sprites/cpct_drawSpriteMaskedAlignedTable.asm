;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawSpriteMaskedAlignedTable
;;
;;    Draws an sprite to video memory (or to a screen buffer), making use of a 
;; given 256-bytes aligned mask table to create transparencies. 
;;
;; C Definition:
;;    void <cpct_drawSpriteMaskedAlignedTable> (const void* *psprite*, void* *pvideomem*, 
;;                                       <u8> *width*, <u8> *height*, 
;;                                       const void* *pmasktable0*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B  BC) psprite     - Source Sprite Pointer
;;  (2B  DE) pvideomem   - Destination video memory pointer
;;  (1B IXL) width       - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B IXH) height      - Sprite Height in bytes (>0)
;;  (2B  HL) pmasktable0 - Pointer to an Aligned Mask Table for transparencies with palette index 0
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteMaskedAlignedTable_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the 
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*. 
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 
;; (<cpct_px2byteM1>) as for mode 2 is linear (1 bit = 1 pixel). 
;;  * *pvideomem* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *width* must be the width of the sprite *in bytes* and must be 1 or more. 
;; Using 0 as *width* parameter for this function could potentially make the program hang 
;; or crash. Always remember that the *width* must be expressed in bytes and *not* in pixels. 
;; The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *pmasktable0* must be a pointer to the start of a mask table that will be used 
;; for calculating transparency. This table *must* consider palette index 0 as transparent.
;; The mask table is expected to be 256-sized containing all the possible masks for each 
;; possible byte-sized colour value. Also, the mask is required to be 256-byte aligned, 
;; which means it has to start at a 0x??00 address in memory to fit in a complete 256-byte 
;; memory page. <cpct_transparentMaskTable00M0> is an example table you might want to use.
;;
;; Known limitations:
;;    * *width*s or *height*s of 0 will be considered as 256 and will potentially 
;; make your program behave erratically or crash.
;;    * Transparent drawing will only work if *pmasktable0* uses *palette index 0*
;; as transparent colour. 
;;    * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;    * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;    * This function *cannot be run from ROM* as it uses self-modifying code.
;;    * If mask table is not aligned within a memory page (256-bytes
;; aligned), rubbish may appear on the screen.
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Details:
;;    This function draws a generic *width* x *height* bytes sprite either to
;; video memory or to a back buffer, creating transparencies using a given
;; mask table. In order to create a valid mask table for this function
;; you may use <cpctm_createTransparentMaskTable> macro.
;; 
;;    The original sprite must be stored as an array (i.e. with 
;; all of its pixels stored as consecutive bytes in memory). It works in
;; a similar way to <cpct_drawSpriteMasked>, but taking care about transparency
;; information. For detailed information about how sprite copying works, 
;; and how video memory is formatted, take a look at <cpct_drawSprite> and
;; <cpct_drawSpriteMasked>.
;;
;;    The way if works is by getting sprite bytes one by one, operating
;; with them, and copying them to video memory (or backbuffer). Each 
;; byte got is used as index to retrieve the associated mask value from
;; the mask table. Then, an AND operation between the byte and the mask
;; is done to remove (set to 0) background pixels. After that, an OR
;; operation between the new byte information and the background (the
;; present byte at video memory location where we want to write) is
;; performed. That effectively mixes sprite colours with background
;; colours, after removing background pixels from the sprite.
;;
;;    *Important*; this process for mixing background with sprite colours works
;; only for *palette index 0* as transparent colour. 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 59 bytes
;;  ASM-bindings - 45 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |    microSecs (us)        |    CPU Cycles
;; ----------------------------------------------------------------
;;  Best      |  28 + (22 + 19W)H + 10HH | 102 + (88 + 76W)H + 40HH
;;  Worst     |       Best + 10          |      Best + 40
;; ----------------------------------------------------------------
;;  W=2,H=16  |        998 / 1008        |    3992 /  4032
;;  W=4,H=32  |       3194 / 3204        |   12776 / 13816
;; ----------------------------------------------------------------
;; Asm saving |          -32             |       -128
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Save Width in a placeholder for easy recovering it later
   ld__a_ixl               ;; [2] A = IXL = width of the sprite
   ld (restore_ixl + 2), a ;; [4] Save IXL (widht of the sprite) in a placeholder for recovering it later

dms_sprite_height_loop:
   push de         ;; [4] Save DE for later use (jump to next screen line)

dms_sprite_width_loop:
   ld    a, (bc)   ;; [2] Get next byte from the sprite
   ld    l, a      ;; [1] Access mask table element (table must be 256-byte aligned)
   ld    a, (de)   ;; [2] Get the value of the byte of the screen where we are going to draw
   and (hl)        ;; [2] Erase background part that is to be overwritten (Mask step 1)
   or    l         ;; [1] Add up background and sprite information in one byte (Mask step 2)
   ld  (de), a     ;; [2] Save modified background + sprite data information into memory
   inc  de         ;; [2] Next bytes (sprite and memory)
   inc  bc         ;; [2] Next byte from the sprite (must be 256-bytes aligned)

   dec__ixl        ;; [2] IXL holds sprite width, we decrease it to count pixels in this line.
   jr   nz,dms_sprite_width_loop;; [2/3] While not 0, we are still painting this sprite line 
                                ;;      - When 0, we have to jump to next pixel line

   pop  de         ;; [3] Recover DE from stack. We use it to calculate start of next pixel line on screen
                   ;;  ... It must be restored before exiting the function, or it will be taken as return address

   dec__ixh        ;; [2] IXH holds sprite height. We decrease it to count another pixel line finished
   jr    z,dms_sprite_copy_ended;; [2/3] If 0, we have finished the last sprite line.
                                ;;      - If not 0, we have to move pointers to the next pixel line
restore_ixl:
   ld__ixl #00     ;; [3] IXL = Restore IXL to the width of the sprite (00 is a placeholder)

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
;; Return is included in bindings