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
;; Function: cpct_drawToSpriteBufferMaskedAlignedTable
;;
;; Draws an sprite to back buffer, making use of a 
;; given 256-bytes aligned mask table to create transparencies. 
;;
;; C Definition:
;;    void <cpct_drawToSpriteBufferMaskedAlignedTable> (<u16> *buffer_width*, void* *inbuffer_ptr*, 
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
;;     * This function *will not work from ROM*, as it uses self-modifying code.
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
;; buffer-memory or another sprite. Both origin and destination sprites must be 
;; stored as arrays (i.e. with all of their pixels stored as consecutive bytes 
;; in memory). It works in a similar way to <cpct_drawSpriteMaskedAlignedTable>, 
;; but drawing to a linearly-disposed memory (as in the case of a sprite array), 
;; instead of a screen video memory formatted one. For detailed information about 
;; how sprite copying works, and how video memory is formatted, take a look at 
;; <cpct_drawSprite> and <cpct_drawSpriteMasked> also.
;;
;;    The way it works is by copying bytes one by one from the origin sprite to
;; the exact destination location inside the sprite-buffer, marked by the pointer
;; *inbuffer_ptr*. The operation performed is not a raw copy, but a copy with 
;; transparency modification. An aligned in-memory transparency table is used
;; to perform and AND operation with bytes from the origin sprite, to remove
;; background pixels from them (palette colour index 0). Afterwards, an OR
;; operation between now transparent bytes and the destination sprite buffer 
;; is performed to mix both of them. Both operations effectively mix sprite 
;; colours (origin sprite) with background colours (destination sprite buffer), 
;; after removing background pixels from the origin sprite.
;;
;;    Next code example shows how to use this function to draw a sprite into
;; a sprite back buffer,
;; (start code)
;;  // Constants defining sizes and locations
;;  #define  SPRBUFFER_W    60 // Width of the sprite buffer IN BYTES
;;  #define  SPRBUFFER_H   100 // Height of the sprite buffer in pixels (same as in bytes)
;;  #define  G_TITLE_W      50 // Width of the sprite of the title IN BYTES
;;  #define  G_TITLE_H      12 // Height of the sprite of the title in pixels (same as in bytes)
;;  
;;  /////////////////////////////////////////////////////////////////////////////////
;;  // Mask Table Definition for Mode 1
;;  //    This creates a 256-bytes transparency mask table that spans from 
;;  // 0x0100 to 0x01FF, making it 256-bytes aligned (high-order byte 0x01 does 
;;  // never change). This table considers any Mode1 0-coloured pixel as transparent
;;  //
;;  cpctm_createTransparentMaskTable(gMaskTable, 0x0100, M1, 0);
;;  
;;  // Sprite declarations
;;  extern u8* g_title;  // g_title sprite is defined in an external file
;;  
;;  // .... More code goes here ....
;;  
;;  // Array used as sprite back buffer. Drawing is performed inside here first.
;;  // After all drawings are performed, this sprite can be drawn in 1 single step 
;;  // to the actual screen video memory
;;  u8 gSpBackBuffer[SPRBUFFER_W * SPRBUFFER_H];
;;  
;;  /////////////////////////////////////////////////////////////////////////////////
;;  // DRAW TITLE
;;  //    Draws the title sprite over a background at the given (tx,ty) location inside
;;  // the sprite back buffer that simulates the viewport. The title sprite has its
;;  // transparent zone coloured as palette index 0. Therefore, we use a transparent
;;  // mask table to make all 0-coloured pixels transparent when drawing.
;;  //
;;  void drawTitle(u8 tx, u8 ty) {
;;     // Get a pointer to the byte inside the Sprite back buffer that is located
;;     // at coordinates (tx, ty) (with respect to the origin of the sprite back buffer)
;;     u8* drawPtr = cpctm_spriteBufferPtr(gSpBackBuffer, SPRBUFFER_W, tx, ty);
;;  
;;     // Draw the title sprite at (tx, ty) coordinates inside the Sprite back buffer
;;     cpct_drawToSpriteBufferMaskedAlignedTable(SPRBUFFER_W, drawPtr, G_TITLE_W, G_TITLE_H, g_title, gMaskTable);
;;  }
;;  
;;  // .... More code goes here ....
;;  
;;  ////////////////////////////////////////////////////////////////////////////////////////////
;;  // DRAW SPRITE BACK BUFFER TO SCREEN
;;  //
;;  //    Waits for VSYNC and then copies the Sprite Back Buffer at the start of the screen
;;  // video memory (actually, it draws it there)
;;  //
;;  void DrawSpriteBackBufferToScreen() {
;;     // Wait for VSYNC and perform the actual drawing of the Sprite
;;     cpct_waitVSYNC();
;;     cpct_drawSprite(gSpBackBuffer, CPCT_VMEM_START, SPRBUFFER_W, SPRBUFFER_H);
;;  }
;;  
;;  // .... More code goes here ....
;;  
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
;;    C-call   - AF, BC, DE, HL
;;    ASM-call - AF, BC, DE, HL, IX
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