;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 Bouche Arnaud
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
;; Function: drawBackBufferMaskedAlignedTable
;;
;; Draws an sprite to back buffer, making use of a 
;; given 256-bytes aligned mask table to create transparencies. 
;;
;; C Definition:
;;    void <cpct_drawBackBufferMaskedAlignedTable> (<u16> *buffer_width*, void* *inbuffer_ptr*, 
;;                                           <u8> *width*, <u8> *height*, void* *sprite*, 
;;                                           <u8>* *mask_table*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;    (1B A)   buffer_width - Width in bytes of the Sprite used as Buffer (>0, >=width)
;;    (2B DE)  inbuffer_ptr - Destination pointer (pointing inside sprite buffer)
;;    (1B IXL) width        - Sprite Width in bytes (>0)
;;    (1B IXH) height       - Sprite Height in bytes (>0)
;;    (2B BC)  sprite       - Source Sprite Pointer (array with pixel data)
;;    (2B HL)  mask_table   - Pointer to the aligned mask-table used to create transparency
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 
;; (<cpct_px2byteM1>) as for mode 2 is linear (1 bit = 1 pixel).
;;  * *width* must be the width of the sprite *in bytes*, the width must be 
;; expressed in bytes and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *buffer* could be any place in memory or in sprite
;;  * *buffer_width* must be the width of the buffer *in bytes*
;;  * *pmasktable* must be a pointer to the mask table that will be used for calculating
;; transparency. A mask table is expected to be 256-sized containing all the possible
;; masks for each possible byte colour value. Also, the mask is required to be
;; 256-byte aligned, which means it has to start at a 0x??00 address in memory to
;; fit in a complete 256-byte memory page. <cpct_transparentMaskTable00M0> is an 
;; example table you might want to use.
;; 
;; Known limitations:
;;     * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of the buffer or the sprite
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside buffer boundaries.
;;     * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;
;; Details:
;;    This function copies a generic WxH bytes sprite from memory to a 
;; buffer-memory or another sprite. The original sprite must be stored as an array (i.e. with 
;; all of its pixels stored as consecutive bytes in memory).
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
;; Destroyed Register values:
;;       AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings   -  53 bytes
;;    ASM-bindings -  37 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      |   microSecs (us)   |     CPU Cycles
;;  -----------------------------------------------------
;;   Any       |  48 + (14 + 19W)H  |  192 + (56 + 76W)H
;;  -----------------------------------------------------
;;   W=2,H=16  |         880        |         3520
;;   W=4,H=32  |        2928        |        11712
;;  -----------------------------------------------------
;;  Asm saving |         -37        |         -148
;;  -----------------------------------------------------
;; (end code)
;;  W = Sprite width in bytes
;;  H = Sprite height in bytes
;;
;; Credits:
;;    Original routine was discussed and developed in CPCWiki by @Docent, 
;;    @Xifos, @demoniak and @Arnaud. Thanks to all of them for their help and support,
;;
;;    http://www.cpcwiki.eu/forum/programming/help-for-speed-up-to-copy-sprite-array-to-another/
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Calculate offset to be added to Destiny pointer (DE, BackBuffer Pointer)
   ;; After copying each sprite line, to point to the start of the next line
   sub__ixl                      ;; [2] A = Back_Buffer_Width - Sprite Width
   ld (offset_to_next_line), a   ;; [4] Modify the offset size inside the copy loop

   ;; Update sprite_width's placeholder inside the copy_loop. This placeholder
   ;; is meant to restore sprite_width into IXL at the start of every iteration,
   ;; before it is used as counter and depleted
   ld__a_ixl               ;; [2] A = Sprite Width (To be saved )
   ld (sprite_width), a    ;; [4] Save sprite width into its copy_loop placeholder

   ;; Perform the copy	 
   copy_loop:
     sprite_width = .+2 ;; Placeholder location for sprite_width
      ld__ixl  #00      ;; [3] IXL = Sprite Width (#00 is a placeholder)

      line_loop:
         ld    a, (bc)  ;; [2] Get next byte from the sprite
         ld    l, a     ;; [1] Access mask table element (table must be 256-byte aligned)
         ld    a, (de)  ;; [2] Get the value of the byte of the screen where we are going to draw
         and (hl)       ;; [2] Erase background part that is to be overwritten (Mask step 1)
         or    l        ;; [1] Add up background and sprite information in one byte (Mask step 2)
         ld  (de), a    ;; [2] Save modified background + sprite data information into memory
         inc  de        ;; [2] Next bytes (sprite and memory)
         inc  bc        ;; [2] Next byte from the sprite (must be 256-bytes aligned)

         dec__ixl       ;; [2] IXL holds sprite width, we decrease it to count pixels in this line.
      jr nz, line_loop  ;; [2/3] While not 0, we are still painting this sprite line 

      ;; Update the Destiny Pointer. DE must point to the place where the
      ;; next sprite line will be copied. So we have to add Backbuffer Width - Sprite Width
     offset_to_next_line = .+1
      ld   a, #00       ;; [2] A = Offset = Backbuffer Width - Sprite Width (00 is a placeholder that gets modified)
      add_de_a          ;; [5] DE += Offset. Make Destiny Pointer point to start of next line

      dec__ixh          ;; [2] One less iteration to complete Sprite Height
   jr nz, copy_loop     ;; [2/3] Repeat copy_loop if IXH!=0 (Iterations pending)

   ;; Ret is included in C/ASM bindings files