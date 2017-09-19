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
;; Function: cpct_drawSpriteClippedMaskedAlignedTable
;;
;;    Copies partially a sprite from an array to video memory (or to a screen buffer).
;;
;; C Definition:
;;    void <cpct_drawSpriteClippedMaskedAlignedTable> (u8 width_to_draw, void *sprite, void* memory, 
;;                                                        u8 width, u8 height, void *mask_table) __z88dk_callee;
;;
;; Input Parameters (9 bytes):
;;  (1B  A') width_to_draw - Sprite Width to Draw in *bytes* (>0 && <= Sprite Width) (Beware, *not* in pixels!)
;;  (2B  BC) psprite       - Source Sprite Pointer
;;  (2B  DE) pvideomem     - Destination video memory pointer
;;  (1B IXL) width         - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B IXH) height        - Sprite Height in bytes (>0)
;;  (2B  HL) pmasktable    - Pointer to the aligned mask table used to create transparency
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteClippedMaskedAlignedTable_asm
;;
;; Parameter Restrictions:
;;  * *width_to_draw* must be the width *in bytes* to draw partially the sprite  
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
;;  * *pmasktable* must be a pointer to the mask table that will be used for calculating
;; transparency. A mask table is expected to be 256-sized containing all the possible
;; masks for each possible byte colour value. Also, the mask is required to be
;; 256-byte aligned, which means it has to start at a 0x??00 address in memory to
;; fit in a complete 256-byte memory page. <cpct_transparentMaskTable00M0> is an 
;; example table you might want to use.
;;
;; Known limitations:
;;    * See <cpct_drawSpriteMaskedAlignedTable>
;;
;; Details:
;;    * See <cpct_drawSpriteMaskedAlignedTable>
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
;;  Best      |  21 + (81 + 18W)H + 10HH | 84 + (324 + 72W)H + 40HH
;;  Worst     |       Best + 10          |     Best + 40
;; ----------------------------------------------------------------
;;  W=2,H=16  |         1893             |       7572
;;  W=4,H=32  |         4917             |      19668
;; ----------------------------------------------------------------
;; Asm saving |          -16             |       -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;; Save Width in a placeholder for easy recovering it later
    ld__a_ixl               ;; [2] A = IXL = width of the sprite
    push bc                 ;; [4] Save BC before computation
   
    ;; Compute Sprite offset X  
    ex af,af'               ;; [1] Sprite Width To Draw = AF <-> A'F'
    ld c, a				    ;; [1] C = Sprite Width To Draw (A')
    ex af,af'               ;; [1] Sprite Width = A'F' <-> A'F'
	sub c 		            ;; [1] A = Sprite Offset (Sprite Width (A) - Sprite Width to Draw (C))
	ld (offset_sprite), a   ;; [2] Set Sprite Offset at placeholder
	
	ld a, c					;; [1] A = Sprite Width to Draw (C)
	ld__ixl_a				;; [2] IXL = Sprite Width to Draw (A)
	ld (restore_ixl + 2), a ;; [4] Save IXL (widht of the sprite) in a placeholder for recovering it later
   
    pop bc                  ;; [3] Recover BC Sprite Width / Height
   
dms_sprite_height_loop:
   push de                  ;; [4] Save DE for later use (jump to next screen line)

dms_sprite_width_loop:
   ld    a, (bc)            ;; [2] Get next byte from the sprite
   ld    l, a               ;; [1] Access mask table element (table must be 256-byte aligned)
   ld    a, (de)            ;; [2] Get the value of the byte of the screen where we are going to draw
   and (hl)                 ;; [2] Erase background part that is to be overwritten (Mask step 1)
   or    l                  ;; [1] Add up background and sprite information in one byte (Mask step 2)
   ld  (de), a              ;; [2] Save modified background + sprite data information into memory
   inc  de                  ;; [2] Next bytes (sprite and memory)
   inc  bc                  ;; [2] Next byte from the sprite (must be 256-bytes aligned)

   dec__ixl                 ;; [2] IXL holds sprite width to draw, we decrease it to count pixels in this line.
   jr   nz,dms_sprite_width_loop ;; [2/3] While not 0, we are still painting this sprite line 
                                 ;;      - When 0, we have to jump to next pixel line

   pop  de                  ;; [3] Recover DE from stack. We use it to calculate start of next pixel line on screen
                            ;;  ... It must be restored before exiting the function, or it will be taken as return address

   dec__ixh                 ;; [2] IXH holds sprite height. We decrease it to count another pixel line finished
   jr    z,dms_sprite_copy_ended;; [2/3] If 0, we have finished the last sprite line.
                                ;;      - If not 0, we have to move pointers to the next pixel line								

   ;; Add Sprite Offset of current sprite data position	
   push hl                    ;; [4] Save hl (background)
   ld h, b                    ;; [1] hl = bc (sprite)
   ld l, c                    ;; [1]
	 
   offset_sprite = .+1       ;; Placeholder for the Sprite Offset
     ld c, #00               ;; [2] C = Sprite Offset 0 is default value
   
   ld  b, #00                ;; [1] Clear B for computation
   add hl, bc                ;; [3] Add offset to sprite index
   ld b, h                   ;; [1] bc = hl (sprite)
   ld c, l                   ;; [1]
   pop hl                    ;; [3] Restore hl (background)

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
