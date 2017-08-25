;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 Bouche Arnaud
;;  Copyright (C) 2017 @Docent / CPCWiki
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
;; Function: cpct_drawToSpriteBuffer
;;
;;    Draws an sprite into another sprite. This lets using the destination sprite
;; as a temporary screen back buffer.
;;
;; C Definition:
;;    void <cpct_drawToSpriteBuffer> (<u16> *buffer_width*, void* *buffer*, <u8> *height*, <u8> *width*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;    (2B HL) sprite      - Source Sprite Pointer (array with pixel data)
;;    (2B DE) buffer      - Destination buffer sprite pointer (also an array)
;;    (1B A) height       - Sprite Height in bytes (>0)
;;    (1B C) width        - Sprite Width in bytes (>0)
;;    (1B B) buffer_width - Width in bytes of the Sprite used as Buffer (>0, >=width)
;;
;; Assembly Call (Input parameters on Registers)
;;    > call cpct_drawToSpriteBuffer_asm
;;
;; Parameter Restrictions:
;;  * *buffer_width* must be greater or equal than *width*. Drawing a sprite into  
;; a buffer sprite that is shorter will probably cause sprite lines to be displaced,
;; and can potentially cause random memory outside buffer to be overwriten, leading
;; to unforeseen consequencies.
;;  * *sprite* must be a pointer to an array containing sprite's pixels data in 
;; screen pixel format. Sprite must be rectangular and all bytes in the array must 
;; be consecutive pixels, starting from top-left corner and going left-to-right, 
;; top-to-bottom down to the bottom-right corner. Total amount of bytes in pixel 
;; array should be *width* x *height*. You may check screen pixel format for 
;; mode 0 (<cpct_px2byteM0>) and mode 1 (<cpct_px2byteM1>) as for mode 2 is 
;; linear (1 bit = 1 pixel).
;;  * *buffer* must be a pointer to an array with same restrictions as *sprite*.
;;  * *width* must be the width of the sprite *in bytes*, the width must be 
;; expressed in bytes and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *buffer_width* must be the width of the sprite used as buffer *in bytes*, 
;;  must be greater than 0 and greater or equal than *width*.
;; 
;; Known limitations:
;;     * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of the buffer or the sprite it might 
;; potentially overwrite memory locations beyond boundaries. In particular, pay 
;; attention to the heights of the sprites and the place where the sprite is 
;; going to be drawn, to ensure that last lines of the sprites are not "drawn" 
;; outside the buffer's memory. This could cause your program to behave erratically, 
;; hang or crash. Always take the necessary steps  to guarantee that you are 
;; drawing inside buffer boundaries.
;;     * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;
;; Details:
;;    This function copies a generic WxH bytes sprite from memory to a 
;; buffer-memory or another sprite.  The original sprite must be stored as 
;; an array (i.e. with all of its pixels stored as consecutive bytes in memory).
;; It works in a similar way to <cpct_drawSprite>, but taking into account that
;; destination memory (the back-buffer sprite) will be arranged linear, with 
;; *buffer_width* offset between lines. This makes it useful to compose 
;; sprites before drawing them to the screen, and also using sprites as
;; back-buffer instead of a hardware back-buffer. For detailed information 
;; about how sprite copying works,  and how video memory is formatted, 
;; take a look at <cpct_drawSprite>.
;;
;; Destroyed Register values:
;;       AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings   - 30 bytes
;;    ASM-bindings - 24 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      |   microSecs (us)   |     CPU Cycles
;;  -----------------------------------------------------
;;   Any       |  24 + (12 + 6W)H   | 96 + (48 + 24W)H
;;  -----------------------------------------------------
;;   W=2,H=16  |         408        |        1632
;;   W=4,H=32  |        1176        |        4704
;;  -----------------------------------------------------
;;  Asm saving |         -19        |        -76
;;  -----------------------------------------------------
;; (end code)
;;  W = Sprite width in bytes
;;  H = Sprite height in bytes
;;
;; Credits:
;;    Original routine was discussed and developed in CPCWiki by @Docent and @Arnaud,
;;
;;    http://www.cpcwiki.eu/forum/programming/help-for-speed-up-to-copy-sprite-array-to-another/
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Calculate offset to be added to Destiny pointer (DE, BackBuffer Pointer)
   ;; After copying each sprite line, to point to the start of the next line
   sub b                         ;; [1] A = Back_Buffer_Width - Sprite Width
   ld (offset_to_next_line), a   ;; [4] Modify the offset size inside the copy loop

   ;; Set the sprite with inside the loop to be restored
   ;; into BC (counter) previous to starting the copy of every line
   ld  a, b                ;; [1] A = Sprite Width
   ld (sprite_width), a    ;; [4] Set the sprite width inside the copy loop

   ;; A Holds the Height of the sprite to be used as counter for the
   ;; copy loop. There will be as many iterations as Height lines
   ld  a, c       ;; [1] A = Sprite Height

   ;; BC will hold either the offset from the end of one line to the
   ;; start of the other, or the width of the sprite. None of them
   ;; will be greater than 256, so B will always be 0.
   ld  b, #00     ;; [2] Set B to 0 so as BC holds the value of C 
      
   ;; Perform the copy
   copy_loop:
      ;; Make BC = sprite width to use it as counter for LDIR,
      ;; which will copy next sprite line
      sprite_width = .+1
      ld   c, #00    ;; [2] BC = Sprite Width (B is always 0, and 00 is a placeholder that gets modified)

      ;; Copy next sprite line to the sprite buffer
      ldir           ;; [6*C-1] Copy one whole line of bytes from sprite to backbuffer
      
      ;; Update the Destiny Pointer. DE must point to the place where the
      ;; next sprite line will be copied. So we have to add Backbuffer Width - Sprite Width
      ex  de, hl     ;; [1] HL holds temporarily the Destiny Pointer (points to backbuffer)
                     ;;     Only for math purposes
      offset_to_next_line = .+1
      ld   c, #00    ;; [2] BC = Offset = Backbuffer Width - Sprite Width (00 is a placeholder that gets modified)
      add hl, bc     ;; [3] Add the offset to the Destiny Pointer (BackBuffer Pointer)
      ex  de, hl     ;; [1] Restore the Destiny Pointer to DE (and HL to what it was)
      
      dec  a         ;; [1]   One less iteration to complete Sprite Height
   jr  nz, copy_loop ;; [2/3] Repeat copy_loop if A!=0 (Iterations pending)

   ret               ;; [3] Return to the caller
      