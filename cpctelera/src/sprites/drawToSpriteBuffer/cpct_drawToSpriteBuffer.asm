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
;; Function: cpct_drawToSpriteBuffer
;;
;;    Draws an sprite inside another sprite's buffer. This permits using the 
;; destination sprite as a temporary screen back buffer.
;;
;; C Definition:
;;    void <cpct_drawToSpriteBuffer> (<u16> *buffer_width*, void* *inbuffer_ptr*, 
;;                                     <u8> *width*, <u8> *height*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;    (1B B)  buffer_width - Width in bytes of the Sprite used as Buffer (>0, >=width)
;;    (2B DE) inbuffer_ptr - Destination pointer (pointing inside sprite buffer)
;;    (1B C)  width        - Sprite Width in bytes (>0)
;;    (1B A)  height       - Sprite Height in bytes (>0)
;;    (2B HL) sprite       - Source Sprite Pointer (array with pixel data)
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
;;  * *inbuffer_ptr* must be a pointer to the place where *sprite* will be drawn 
;; inside the sprite buffer. It can point to any of the bytes in the array of
;; the destination sprite buffer. That will be the place where the first byte
;; of the *sprite* will be copied to (its top-left corner). It is important to
;; check that there is enough space for the sprite to be copied from that byte on.
;; Otherwise, the copy loop will continue outside the sprite buffer boundaries.
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
;;     * This function *will not work from ROM*, as it uses self-modifying code.
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
;; buffer-memory or another sprite.  Both the sprite to-be-copied and the
;; destination-sprite (the sprite-buffer) must be linear arrays (i.e. with 
;; all of their pixels stored as consecutive bytes in memory). It works in 
;; a similar way to <cpct_drawSprite>, but taking into account that
;; destination memory (the sprite-buffer) will be arranged linear, with 
;; *buffer_width* offset between lines. This makes it useful to compose 
;; sprites before drawing them to the screen, and also using sprites as
;; back-buffer instead of a hardware back-buffer. For detailed information 
;; about how sprite copying works,  and how video memory is formatted, 
;; take a look at <cpct_drawSprite>.
;;
;;    This code shows a great example of what can be done with this function,
;; (start code)
;;  // Size of the Background 
;;  #define BACK_W  60
;;  #define BACK_H 100
;;
;;  // Size of Entities
;;  #define ENT_W    4
;;  #define ENT_H    8
;;
;;  // Redraws the action screen. It first draws Main Character and Enemy
;;  // over the Background Sprite, then draws the composite background
;;  // sprite to the screen after waiting to VSYNC. This sequence
;;  // minimizes the amount of data to be writen to the screen after
;;  // waiting to VSYNC, eliminating flicking and tearing.
;;  void redrawActionScreen(u8 *scr_p, TEntity *en, TEntity *ch) {
;;      cpct_drawToSpriteBuffer(BACK_W, g_background + ch->y*BACK_W + ch->x, ENT_W, ENT_H, g_character);
;;      cpct_drawToSpriteBuffer(BACK_W, g_background + en->y*BACK_W + en->x, ENT_W, ENT_H, g_enemy);
;;      cpct_waitVSync();
;;      cpct_drawSprite(g_background, scr_p, BACK_W, BACK_H);
;;  }
;; (end code)
;;     
;;     Drawing to sprites instead of the screen lets us do as many draw 
;; operations as required without worrying about the raster and flickering
;; or tearing effects. As nothing is being changed in video memory, no 
;; problematic effects are produced. Once the image is composed in one or 
;; a few sprites, these can be drawn to the screen. This minimizes the 
;; total cycles required to copy data from memory to video memory.
;;
;;         Also, as destination sprite is a normal sprite with its data 
;; distributed linear in memory, calculating a position inside the sprite 
;; is easier than in video memory. It only requires multiplying the 
;; y-coordinate by the width of the sprite-buffer (to jump from its start 
;; point to the y-th line), then adding the x-coordinate. Everything is 
;; also added to the starting point of the sprite buffer. Moreover, this 
;; calculations can be easily sped up by carefully selecting the width 
;; of the sprite-buffer. If it is a power of 2, then multiplications will 
;; become simple shifts, speeding up the proccess.
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

   ;; Set the sprite with inside the loop to be restored
   ;; into BC (counter) previous to starting the copy of every line
   ld  a, c                        ;; [1] A = Sprite Width
   ld (sprite_width_restore), a    ;; [4] Set the sprite width inside the copy loop

   ;; A Holds the Height of the sprite to be used as counter for the
   ;; copy loop. There will be as many iterations as Height lines
   ld  a, b       ;; [1] A = Sprite Height

   ;; BC will hold either the offset from the end of one line to the
   ;; start of the other, or the width of the sprite. None of them
   ;; will be greater than 256, so B will always be 0.
   ld  b, #00     ;; [2] Set B to 0 so as BC holds the value of C 
      
   ;; Perform the copy
   copy_loop:
      ;; Make BC = sprite width to use it as counter for LDIR,
      ;; which will copy next sprite line
      sprite_width_restore = .+1
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
