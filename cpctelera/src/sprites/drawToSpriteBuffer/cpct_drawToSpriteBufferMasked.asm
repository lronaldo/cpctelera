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
;; Function: cpct_drawToSpriteBufferMasked
;;
;;    Draws an sprite inside another sprite's buffer, using mask as transparency 
;; information to prevent erasing the background. This permits using the destination 
;; sprite as a temporary screen back buffer.
;; 
;; C Definition:
;;    void <cpct_drawToSpriteBufferMasked> (<u16> *buffer_width*, void* *inbuffer_ptr*, 
;;                                           <u8> *width*, <u8> *height*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;    (1B B)  buffer_width - Width in bytes of the Sprite used as Buffer (>0, >=width)
;;    (2B DE) inbuffer_ptr - Destination pointer (pointing inside sprite buffer)
;;    (1B C)  width        - Sprite Width in bytes (>0)
;;    (1B A)  height       - Sprite Height in bytes (>0)
;;    (2B HL) sprite       - Source Sprite Pointer (array with pixel data along with mask data)
;;
;; Assembly Call (Input parameters on Registers)
;;    > call cpct_drawToSpriteBufferMasked_asm
;;
;; Parameter Restrictions:
;;  * *buffer_width* must be greater or equal than *width*. Drawing a sprite into  
;; a buffer sprite that is shorter will probably cause sprite lines to be displaced,
;; and can potentially cause random memory outside buffer to be overwriten, leading
;; to unforeseen consequencies.
;;  * *inbuffer_ptr* must be a pointer to the place where *sprite* will be drawn 
;; inside the sprite buffer. It can point to any of the bytes in the array of
;; the destination sprite buffer. That will be the place where the first byte
;; of the *sprite* will be copied to (its top-left corner). It is important to
;; check that there is enough space for the sprite to be copied from that byte on.
;; Otherwise, the copy loop will continue outside the sprite buffer boundaries.
;;  * *width* must be the width of the sprite *in bytes*, *excluding mask data*, and 
;; must be 1 or more. Using 0 as *width* parameter for this function could potentially 
;; make the program hang or crash. Always remember that the *width* must be 
;; expressed in bytes and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; along with mask data, take a look at <cpct_drawSpriteMasked>.
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
;; buffer-memory or another sprite. Both origin and destination sprites must be 
;; stored as arrays (i.e. with all of their pixels stored as consecutive bytes 
;; in memory). It works in a similar way to <cpct_drawSpriteMasked>, but drawing 
;; to a linearly-disposed memory (as in the case of a sprite array), instead 
;; of a screen video memory formatted one.
;;
;;    The function copies bytes one by one from the origin sprite to the 
;; place within the destination sprite marked by *inbuffer_ptr*. The process
;; does a "blending" instead of a raw copy. The blending depends on the mask
;; that must be included within the origin sprite, interlaced with its colour
;; pixel data (in the same format as for <cpct_drawSpriteMasked>). The blending
;; consist in applying the mask to the bytes from the origin sprite (AND 
;; operation), then mixing the result with the bytes from the destination
;; sprite (OR operation). This is designed mainly to be used for simulating
;; transparency on the origin sprite.
;;
;;    This code shows a great example of what can be done with this function,
;; (start code)
;; // Constants defining sizes and locations
;; // "Viewport" is the sprite buffer drawn in the screen video memory
;; #define  SPRBUFFER_W    60 // Width of the sprite buffer IN BYTES
;; #define  SPRBUFFER_H    80 // Height of the sprite buffer in pixels (same as in bytes)
;; #define  VIEW_X         10 // X Coordinate of the Viewport (location where sprite buffer is drawn in screen video memory)
;; #define  VIEW_Y         30 // Y Coordinate of the Viewport (location where sprite buffer is drawn in screen video memory)
;; #define  G_SHIP_W       28 // Width of the sprite of the ship IN BYTES
;; #define  G_SHIP_H       20 // Height of the sprite of the ship in pixels (same as in bytes)
;; #define  SHIP_X         12 // X coordinate of the ship inside the sprite buffer (IN BYTES)
;; #define  SHIP_Y         15 // Y coordinate of the ship inside the sprite buffer (in pixels, same as in bytes)
;; // As Viewport location is fixed, we can define it as a pre-calculated constant
;; // We use the calculation macro for this purpose, as it will be compiled to the result.
;; #define  VIEW_PTR       cpctm_screenPtr(CPCT_VMEM_START, VIEW_X, VIEW_Y)
;; 
;; // Array used as sprite back buffer. Drawing is performed inside here first.
;; // After all drawings are performed, this sprite can be drawn in 1 single step 
;; // to the actual screen video memory
;; u8 gSpBackBuffer[SPRBUFFER_W * SPRBUFFER_H];
;; 
;; // .... Other functions go here ....
;; 
;; // This function draws all elements to the sprite back buffer. Afterwards, it
;; // waits for VSYNC and then it draws the sprite back buffer to the screen video memory
;; void  drawEverything() {
;;    u8* drawPtr;
;;    
;;    // .... Other drawings here (tiles, background, sprites...)
;; 
;;    // Calculate exact byte location inside the sprite buffer where our ship
;;    // has to be drawn, according to its coordinates (X, Y). (Coordinates are 
;;    // local to the sprite buffer space, being (0,0) the first byte, up-left corner)
;;    drawPtr = cpctm_spriteBufferPtr(gSpBackBuffer, SPRBUFFER_W, SHIP_X, SHIP_Y);
;;    // Now that we have the exact byte location, draw the
;;    // sprite of the ship to the sprite back buffer
;;    cpct_drawToSpriteBufferMasked(SPRBUFFER_W, drawPtr, G_SHIP_W, G_SHIP_H, g_ship);
;; 
;;    // .... Other drawings here (tiles, background, sprites...)
;; 
;;    // All drawings are finished, wait for VSYNC and then copy 
;;    // the sprite buffer to screen video memory
;;    cpct_waitVSYNC();
;;    cpct_drawSprite(gSpBackBuffer, VIEW_PTR, SPRBUFFER_W, SPRBUFFER_H);   
;; }
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
;;   Any       |  13 + (17 + 12W)H   | 52 + (68 + 48W)H
;;  -----------------------------------------------------
;;   W=2,H=16  |         749        |        2996
;;   W=4,H=32  |        2573        |        10292
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
   sub c                         ;; [1] A = Offset = Back_Buffer_Width - Sprite Width
   ld (offset_to_next_line), a   ;; [4] Modify the offset size inside the copy loop

   ;; Modify the Sprite Width in the placeholder that is located inside the copy loop.
   ;; This lets the copy loop to easily restore the Sprite Width on every iteration
   ld  a, c                      ;; [1] A = Sprite Width
   ld (sprite_width_restore), a  ;; [4] Modify the Sprite Width value inside the copy loop

   ;; Perform the copy
   copy_loop:
      sprite_width_restore = .+1
      ld c, #00         ;; [2] Restore Sprite Width into c to use it as counter for line_loop
                        ;;     (00 is a placedholder that will be self-modified)
      line_loop :
         ld    a ,(de)  ;; [2] Get next background byte into A
         and (hl)       ;; [2] Erase background part that is to be overwritten (Mask step 1)
         inc  hl        ;; [2] HL += 1 => Point HL to Sprite Colour information
         or  (hl)       ;; [2] Add up background and sprite information in one byte (Mask step 2)
         ld  (de), a    ;; [2] Save modified background + sprite data information into memory
         inc  de        ;; [2] Next bytes (sprite and memory)
         inc  hl        ;; [2] 
   
         dec   c        ;; [1]   One less iteration to complete Sprite Width
      jr nz, line_loop  ;; [2/3] Repeat line_loop if C!=0 (Iterations pending)
      
      ;; Update the Destiny Pointer. DE must point to the place where the
      ;; next sprite line will be copied. So we have to add 'Backbuffer Width - Sprite Width'
      ;; which is the offset to the start of the next line to be drawn inside the sprite buffer
      ;;    DE + A (DE=Destiny Pointer inside the sprite buffer, A=Offset to next Line)
      offset_to_next_line = .+1
      ld  a, #00     ;; [2] A = Offset_to_next_line (00 is a place holder that will be self-modified)
      add_de_a       ;; [5] DE += A (Macro)

   djnz copy_loop    ;; [3/4] B--, One less iteration to complete Sprite Height
                     ;;          Repeat copy_loop if B!=0 (Iterations pending)

   ret               ;; [3] Return to the caller
