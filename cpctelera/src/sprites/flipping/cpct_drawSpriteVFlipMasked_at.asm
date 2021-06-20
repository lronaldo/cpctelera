;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawSpriteVFlipMasked_at
;;
;;    Draws a sprite with transparency from an array to video memory or 
;; Hardware Back Buffer flipping it vertically (top to bottom), using a 256-byte 
;; Aligned Transparency Table.
;;
;; C Definition:
;;    void <cpct_drawSpriteVFlipMasked_at> (const void* *sprite*, void* *memory*, 
;;                                           <u8> *width*, <u8> *height*, 
;;                                           const void* *pmasktable0*) __z88dk_callee;
;;
;; Input Parameters (8 bytes):
;;  (2B  BC) sprite      - Source Sprite Pointer
;;  (2B  DE) memory      - Destination video memory pointer (*Bottom-left corner*)
;;  (1B IXL) width       - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (1B IXH) height      - Sprite Height in bytes 
;;  (2B  HL) pmasktable0 - Pointer to an Aligned Mask Table for transparencies with palette index 0
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteVFlipMasked_at_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the 
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*. 
;;  * *memory* must be a pointer to *bottom-left corner* of the place where the sprite 
;; is to be drawn. The sprite will then be drawn bottom-to-top, starting at the 
;; video memory byte *memory* is pointing at. It could be any place in memory, 
;; inside or outside current video memory. It will be equally treated as video 
;; memory (taking into account CPC's video memory disposition). This lets you 
;; copy sprites to software or hardware backbuffers, and not only video memory.
;;  * *width* (1-256) must be the width of the sprite *in bytes*, not accounting
;; for mask bytes. Always remember that the width must be expressed in bytes 
;; and *not* in pixels. 
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
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
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;    * If mask table is not aligned within a memory page (256-bytes
;; aligned), rubbish may appear on the screen.
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Details:
;;    Draws *sprite* to *memory* using *pmasktable0* as mask table for transparency
;; and taking into account CPC's standard video memory disposition. *memory* can be 
;; pointing to actual video memory or to a hardware backbuffer. 
;;
;;    The drawing process uses *pmasktable0* as mask AND-table to produce transparency.
;; Each *sprite* byte to be drawn is used as index to get a mask value from the *pmasktable0*.
;; Then, background is AND-ed with got mask value, and result is OR-ed with the *sprite*
;; byte. AND operation removes background pixels where the sprite will be placed, 
;; then OR operation inserts sprite pixels. This operation is faster than a more general
;; OR-AND-OR operation, but it works *only* with palette index 0 as transparent colour. 
;;
;;    This function works completely similar to <cpct_drawSpriteVFlip> but using *pmasktable0*
;; to draw transparent sprites. For more details on how it works, please check
;; <cpct_drawSpriteVFlip> documentation.
;;
;;    Use example,
;; (start code)
;;    // Define a 256-bytes Transparency Mask Table at 0x100 in memory
;;    // that uses mode 0 - palette index 0 as transparent colour
;;    cpctm_createTransparentMaskTable (gTransparencyTable, 0x0100, M0, 0);
;;
;;    ///////////////////////////////////////////////////////////////////
;;    // DRAW ARROWS
;;    //    Draws arrows shot by enemies. There is only one arrow sprite
;;    // pointing up, but enemies can shoot up or down. Arrows are drawn
;;    // vertically flipped when they are shot down.
;;    //
;;    void drawArrows(Arrow** arrows, u8 totalArrows) {
;;       u8* pvmem;  // Pointer to video memory
;;       u8  i;
;;    
;;       // Draw all arrows
;;       //
;;       for(i=0; i < totalArrows; ++i) {
;;          // Get a pointer to the next arrow to be drawn
;;          Arrow *a = arrows[i];
;;
;;          // Drawing depends on arrow sense. Negative Y velocity
;;          // means going up. Otherwise, it goes down
;;          //
;;          if (a->vy < 0) {
;;             //-----Draw arrow moving up (original sprite sense)
;;             //
;;             // Get a pointer to video memory byte for arrow location and draw the arrow
;;             pvmem = cpct_getScreenPtr(CPCT_VMEM_START, a->x, a->y);
;;             cpct_drawSpriteMaskedAlignedTable(ARROW_SPRITE, pvmem, ARROW_WIDTH, ARROW_HEIGHT, gTransparencyTable);
;;          } else {
;;             //-----Draw arrow moving up (original sprite sense)
;;             //
;;             // We get a pointer to the start of the bottom-left of video memory
;;             // where the arrow sprite has to be draw. That is, at row Y+HEIGHT-1.
;;             // Then, with that pointer, we draw the arrow sprite vertically flipped
;;             pvmem = cpct_getScreenPtr(CPCT_VMEM_START, a->x, (a->y + ARROW_HEIGHT - 1) );
;;             cpct_drawSpriteVFlipMasked_at(ARROW_SPRITE, pvmem, ARROW_WIDTH, ARROW_HEIGHT, gTransparencyTable);
;;          }
;;       }
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;     C-bindings - AF, BC, DE, HL
;;   ASM-bindings - AF, BC, DE, HL, IX
;;
;; Required memory:
;;     C-bindings - 68 bytes
;;   ASM-bindings - 54 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)        |        CPU Cycles
;; -----------------------------------------------------------------
;;  Best      | 25 + (21 + 19W)H + 10HH | 100 + (84 + 76W)H + 40HH
;;  Worst     |       Best + 10         |      Best + 40
;; -----------------------------------------------------------------
;;  W=2,H=16  |        979 /  989       |    3916 /  3956
;;  W=4,H=32  |       3159 / 3169       |   12636 / 12676
;; -----------------------------------------------------------------
;; Asm saving |         -29             |        -116
;; -----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = int((H-1)/8)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Modify code using width inside drawing loops
   ld__a_ixl            ;; [2] A = width
   ld (widthRestore), a ;; [4] Save sprite width into its restoration placeholder for row loop
   neg                  ;; [2] A = -A (0 - Sprite Width)
   ld (minusWidthOp), a ;; [4] Save negative sprite width into placeholder for next row calculation

   ;; Copy a complete sprite row, byte by byte  
nextByte:
   ld    a, (bc)     ;; [2] Get next byte from the sprite
   ld    l, a        ;; [1] Access mask table element (table must be 256-byte aligned)
   ld    a, (de)     ;; [2] Get the value of the byte of the screen where we are going to draw
   and (hl)          ;; [2] Erase background part that is to be overwritten (Mask step 1)
   or    l           ;; [1] Add up background and sprite information in one byte (Mask step 2)
   ld  (de), a       ;; [2] Save modified background + sprite data information into memory
   inc  de           ;; [2] Next bytes (sprite and memory)
   inc  bc           ;; [2] Next byte from the sprite (must be 256-bytes aligned)

   dec__ixl          ;; [2] IXL holds sprite width, we decrease it to count pixels in this line.
   jr   nz, nextByte ;; [2/3] While not 0, we are still painting this sprite line 

   dec__ixh          ;; [2] IXH--; 1 more row of the sprite completed
   jr    z, return   ;; [2/3] If that was the last row, we safely return

widthRestore = .+2
   ld__ixl #00       ;; [3] Restore width value before next_row loop start (00 is a placeholder)

   ;; Jump destination pointer to the start of the next row in video memory
   ld     a, e       ;; [1] DE += -(0x800 + sprite width) => Offset to jump to previous row 
minusWidthOp = .+1
   add   #0x00       ;; [2] 00 is a placeholder for -width value
   ld     e, a       ;; [1] |
   ld     a, d       ;; [1] |
   adc   #0xF7       ;; [2] | We substract 0x800 plus the width of the sprite 
   ld     d, a       ;; [1] | 

   ;; We check if we have crossed video memory boundaries (which will happen every 8 rows). 
   ;; .... If that happens, bits 13,12 and 11 of destination pointer will be 1
   and   #0x38       ;; | [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   xor   #0x38       ;; | [2] ... inverted (so that they 3 turn to 0 if they were 1)
   jr   nz, nextByte ;; [2/3] If any bit from {13,12,11} is not 0, we are still inside 
                     ;; .... video memory boundaries, so proceed with next row

   ;; Every 8 rows, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means we have to advance 16K 
   ;; to return to the video memory and then jump -0x50 back to move to 
   ;; previous row. So, in a single operation, we need to do 
   ;; DE += 0x4000 - 0x50 => DE += 0x3FB0
   ld    a, e      ;; [1] DE -= 0xC050h ( DE += 0x3FB0 )
   add   #0xB0     ;; [2] | Relocate DE pointer to the start of the next pixel line:
   ld    e, a      ;; [1] | DE is moved forward 1 memory bank (16K) minus 50 bytes (0x4000 - 0x50) 
   ld    a, d      ;; [1] | which effectively is the same as restoring it from previous backwards
   adc   #0x3F     ;; [2] | movement then jumping to previous pixel line, only 50 bytes backwards
   ld    d, a      ;; [1] | Calculations are made with 8 bit maths as it is faster than other alternatives here

   jr  nextByte    ;; [3] Jump to continue with next pixel line

return:
   ;; Ret instruction is provided by C/ASM bindings