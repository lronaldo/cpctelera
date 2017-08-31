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
;;                                           <u8> *width*, <u8> *height*, void* *sprite*, <u8>* *mask_table*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;    (1B A)   buffer_width - Width in bytes of the Sprite used as Buffer (>0, >=width)
;;    (2B DE)  inbuffer_ptr - Destination pointer (pointing inside sprite buffer)
;;    (1B C)   width        - Sprite Width in bytes (>0)
;;    (1B B)   height       - Sprite Height in bytes (>0)
;;    (2B HL)  sprite       - Source Sprite Pointer (array with pixel data)
;;    (2B HL') mask_table   - Pointer to the aligned mask table used to create transparency
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
;;       AF, AF', BC, DE, HL, HL'
;;
;; Required memory:
;;    C-bindings   - 30 bytes
;;    ASM-bindings - 24 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      |   microSecs (us)   |     CPU Cycles
;;  -----------------------------------------------------
;;   Any       |  14 + (15 + 20W)H   | 56 + (60 + 80W)H
;;  -----------------------------------------------------
;;   W=2,H=16  |         894        |        3576
;;   W=4,H=32  |        3054        |        12216
;;  -----------------------------------------------------
;;  Asm saving |         -21        |        -84
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
   sub c                         ;; [1] A = Back_Buffer_Width - Sprite Width
   ld (offset_to_next_line), a   ;; [4] Modify the offset size inside the copy loop

   ;; IXL Holds the Width of the sprite
   ld__ixl_c      ;; [3] IXL = Sprite Width

   ;; A Holds the Height of the sprite to be used as counter for the
   ;; copy loop. There will be as many iterations as Height lines
   ld  a, b       ;; [1] A = Sprite Height

   ;; BC will hold either the offset from the end of one line to the
   ;; start of the other, or the width of the sprite. None of them
   ;; will be greater than 256, so B will always be 0.
   ld  b, #00     ;; [2] Set B to 0 so as BC holds the value of C 
	
   ;; Perform the copy	
   copy_loop :
        ;; Make BC = sprite width to use it as counter for line_loop, which will copy next sprite line
	ld__c_ixl   ;; [3] C = IXL = Sprite Width
        ex af, af'  ;; [1] A' <=> A : Switch Sprite Height to empty A' 

	line_loop :
		exx                   ;; [1] HL' <-> HL : Switch to Sprite
		ld	a, (hl);      ;; [2] Get Sprite into A
		inc	hl            ;; [2] HL += 1 => Point HL to Sprite Colour information
		exx                   ;; [1] HL' <-> HL : Switch to Masked Aligned Table
		ld	l, a          ;; [1] Get Sprite into L
		ld	a, (de);      ;; [2] Get next background byte into A
		and (hl)              ;; [2] Erase background part that is to be overwritten (Mask step 1)
		or l;                 ;; [2] Add up background and sprite information in one byte (Mask step 2)
		ld(de), a;            ;; [2] Save modified background + sprite data information into memory
		inc	de            ;; [2] Next bytes (sprite and memory)
		dec 	c             ;; [1] One less iteration to complete Sprite Width
		jr 	nz, line_loop ;; [2/3] Repeat line_loop if C!=0 (Iterations pending)

     ;; Update the Destiny Pointer. DE must point to the place where the
     ;; next sprite line will be copied. So we have to add Backbuffer Width - Sprite Width
    ex  de, hl        ;; [1] HL holds temporarily the Destiny Pointer (points to backbuffer)
		      ;;  Only for math purposes
    offset_to_next_line = .+1
    ld   c, #00       ;; [2] BC = Offset = Backbuffer Width - Sprite Width (00 is a placeholder that gets modified)
    add hl, bc        ;; [3] Add the offset to the Destiny Pointer (BackBuffer Pointer)
    ex  de, hl        ;; [1] Restore the Destiny Pointer to DE (and HL to what it was)
    ex	af, af'       ;; [1] A <=> A' : Switch to Sprite Height A counter
    dec  a            ;; [1] One less iteration to complete Sprite Height
    jr  nz, copy_loop ;; [2/3] Repeat copy_loop if A!=0 (Iterations pending)

   ret                ;; [3] Return to the caller
