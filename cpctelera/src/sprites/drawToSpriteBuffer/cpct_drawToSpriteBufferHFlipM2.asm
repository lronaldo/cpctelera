;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2022 Bouche Arnaud
;;  Copyright (C) 2022 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawToSpriteBufferHFlipM2
;;
;; Draws a Mode 2 sprite from an array inside another sprite's buffer
;; flipping it Horizontally (right to left).
;; This permits using the destination sprite as a temporary screen back buffer.
;;
;; C Definition:
;;    void <cpct_drawToSpriteBufferHFlipM2> (<u16> *buffer_width*, void* *inbuffer_ptr*, 
;;                                           <u8> *width*, <u8> *height*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;    (1B A)  buffer_width - Width in bytes of the Sprite used as Buffer (>0, >=width)
;;    (2B DE) inbuffer_ptr - Destination pointer (pointing inside sprite buffer)
;;    (1B C)  width        - Sprite Width in bytes (>0)
;;    (1B B)  height       - Sprite Height in bytes (>0)
;;    (2B HL) sprite       - Source Sprite Pointer (array with pixel data)
;;
;; Assembly Call (Input parameters on Registers)
;;    > call cpct_drawToSpriteBufferHFlipM2_asm
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
;;      if (ch->dir == LEFT)
;;         cpct_drawToSpriteBuffer(BACK_W, g_background + ch->y*BACK_W + ch->x, ENT_W, ENT_H, g_character);
;;      else
;;         cpct_drawToSpriteBufferHFlipM2(BACK_W, g_background + ch->y*BACK_W + ch->x, ENT_W, ENT_H, g_character);
;;
;;      if (ch->dir == LEFT)
;;         cpct_drawToSpriteBuffer(BACK_W, g_background + en->y*BACK_W + en->x, ENT_W, ENT_H, g_enemy);
;;      else
;;         cpct_drawToSpriteBufferHFlipM2(BACK_W, g_background + en->y*BACK_W + en->x, ENT_W, ENT_H, g_enemy);
;;
;;      cpct_waitVSync();
;;      cpct_drawSprite(g_background, scr_p, BACK_W, BACK_H);
;;  }
;; (end code)
;;
;; See cpct_drawToSpriteBuffer for drawing to buffer operation
;; See cpct_drawSpriteHFlipM2 for flipping operation
;;
;; Destroyed Register values:
;;       AF, BC, DE, HL, IX
;;
;; Required memory:
;;    C-bindings   - 54 bytes
;;    ASM-bindings - 48 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      |   microSecs (us)   |     CPU Cycles
;;  -----------------------------------------------------
;;   Any       |  20 + (13 + 20W)H   | 80 + (52 + 80W)H
;;  -----------------------------------------------------
;;   W=2,H=16  |         868        |        3472
;;   W=4,H=32  |        2996        |       11976
;;  -----------------------------------------------------
;;  Asm saving |         -28        |        -112
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

   ;; Use IXL as counter for sprite rows (B will be used for other operations)
   ld__ixl_b                     ;; [2] IXL = Height
   ex   de, hl                   ;; [1] HL  = Destination Buffer <-> DE = Sprite Pointer
  
   ;; Calculate offset to be added to Destination pointer (HL BackBuffer Pointer)   
   add  c                        ;; [1] A = Back_Buffer_Width + Sprite Width
   ld  (offsetToNextLine), a     ;; [4] Modify the offset size inside the copy loop

   ;; Save C to restore sprite width for each new row
   ld__ixh_c                     ;; [2] IXH = C = Sprite Width 
   
   ;; Make HL point to the right-most byte of the first row of the buffer to be drawn
   ld   a, c                     ;; [1] A = C = width
   dec  a                        ;; [1] A--
   add_hl_a                      ;; [5] HL += width - 1 (Point to last byte at the buffer)

   jr  firstByte                 ;; [3] First byte does not require C to be restored nor DE/HL to be changed
  
;; Perform the copy
nextByte:
   inc  de                       ;; [2] DE++ (Next sprite byte)
   dec  hl                       ;; [2] HL-- (Destination previous byte in buffer memory. We draw right-to-left)
firstByte:
   ld   a, (de)                       ;; [2] A = Next sprite byte
   ld   b, a                          ;; [1] B = A (For temporary calculations)
  cpctm_reverse_mode_1_pixels_of_A b  ;; [7] Reverses both pixels of sprite byte in A
   ld  (hl), a                        ;; [2] Save Sprite byte with both pixels reversed

   dec  c                        ;; [1] C-- (One less byte in this row to go)
   jr   nz, nextByte             ;; [2/3] If C!=0, there are more bytes, so continue with nextbyte
  
  ;; Update the Destination Pointer, so we have to add Backbuffer Width
offsetToNextLine = .+1
  ld    bc, #0000                ;; [3] BC = Offset = Backbuffer Width + Sprite Width (0000 is a placeholder that gets modified)  
  add   hl, bc                   ;; [3] Add the offset to the Destination Pointer (BackBuffer Pointer)
  ld__c_ixh                      ;; [2] Restore Width into C before looping over next row bytes one by one    
  
  dec__ixl                       ;; [2]   IXL-- (One less sprite row to go)
  jr    nz, nextByte             ;; [2/3] Repeat copy_loop if IXL!=0 (Iterations pending)

return:
  ;; Ret instruction provided by C/ASM bindings
