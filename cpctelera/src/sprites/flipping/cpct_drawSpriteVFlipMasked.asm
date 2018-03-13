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
;; Function: cpct_drawSpriteVFlipMasked
;;
;;    Draws a masked sprite from an array to video memory or Hardware Back Buffer 
;; flipping it vertically (top to bottom).
;;
;; C Definition:
;;    void <cpct_drawSpriteVFlipMasked> (void* *sprite*, void* *memory*, <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B DE ) sprite - Source Masked-Sprite Pointer
;;  (2B HL ) memory - Destination video memory pointer (*Bottom-left corner*)
;;  (1B A  ) width  - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (1B IXH) height - Sprite Height in bytes 
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteVFlipMasked_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; along with mask data. Each mask byte will contain enabled bits as those that should
;; be picked from the background (transparent) and disabled bits for those that will
;; be printed from sprite colour data. Each mask data byte must precede its associated
;; colour data byte. Sprite must be rectangular and all bytes in the array must be 
;; consecutive pixels, starting from top-left corner and going left-to-right, 
;; top-to-bottom down to the bottom-right corner. Total amount of bytes in pixel array 
;; should be 2 x *width* x *height* (mask data doubles array size). You may check screen 
;; pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 (<cpct_px2byteM1>) as 
;; for mode 2 is linear (1 bit = 1 pixel).
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
;;
;; Known limitations:
;;    * A sprite *width* of 0 will be considered as 256 and will potentially 
;; make your program behave erratically or crash.
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
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Details:
;;    Draws given masked *sprite* to *memory* taking into account CPC's standard 
;; video memory disposition. Therefore, *memory* can be pointing to actual video memory
;; or to a hardware backbuffer. The drawing assumes that the sprite contains 
;; mask data along with pixel data, and uses mask data to mix sprite pixels with
;; background pixels. Mixing is achieved by AND-ing backgound with mask, and then
;; OR-ing result with the sprite. AND operation removes background pixels where
;; the sprite will be placed, then OR operation inserts sprite pixels.
;;
;;    It works completely similar to <cpct_drawSpriteVFlip> but taking into account
;; the transparency mask. For more details on how it works, please check
;; <cpct_drawSpriteVFlip> documentation.
;;
;;    Use example,
;; (start code)
;;    ///////////////////////////////////////////////////////////////////
;;    // DRAW ENEMY UP-DOWN-UP 
;;    //    Draws an enemy that patrols going up-down-up. The sprite of
;;    // the enemy is looking up, so it has to be drawn vertically 
;;    // flipped when it moves downwards.
;;    //
;;    void drawEnemyUpDownUp(Enemy* e) {
;;       u8* pvmem;  // Pointer to video memory
;;    
;;       // Drawing depends on enemy's moving sense
;;       //
;;       if (e->movingUp) {
;;          //-----Draw enemy moving up
;;          //
;;          // Get a pointer to video memory byte for enemy location
;;          pvmem = cpct_getScreenPtr(CPCT_VMEM_START, e->x, e->y);
;;          // Draw the enemy
;;          cpct_drawSpriteMasked(e->sprite, pvmem, e->width, e->height);
;;       } else {
;;          //-----Draw enemy moving down (vertically flipped)
;;          //
;;          // We need a pointer to the start of the bottom-left of video memory
;;          // where the enemy sprite has to be draw. That is, at row Y+HEIGHT-1.
;;          pvmem = cpct_getScreenPtr(CPCT_VMEM_START, e->x, (e->y + e->height - 1) );
;;          // Finally, draw the enemy vertically flipped. This draw function
;;          // does the drawing bottom-to-top in the video memory. That's the reason
;;          // to have a pointer to the bottom-left.
;;          cpct_drawSpriteVFlipMasked(e->sprite, pvmem, e->width, e->height);
;;       }
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;     C-bindings - AF, BC, DE, HL
;;   ASM-bindings - AF, BC, DE, HL, IXH
;;
;; Required memory:
;;     C-bindings - 55 bytes
;;   ASM-bindings - 43 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;  Best      | 27 + (21 + 18W)H + 8HH | 108 + (84 + 72W)H + 32HH
;;  Worst     |       Best + 8         |      Best + 32
;; ----------------------------------------------------------------
;;  W=2,H=16  |        947 /  955      |    3788 /  3820
;;  W=4,H=32  |       3027 / 3035      |   12108 / 12140
;; ----------------------------------------------------------------
;; Asm saving |         -28            |        -112
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = int((H-1)/8)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Modify code using width inside drawing loops
   ld (widthRestore), a ;; [4] Save sprite width into its restoration placeholder for row loop
   neg                  ;; [2] A = -A (0 - Sprite Width)
   ld (minusWidthOp), a ;; [4] Save negative sprite width into placeholder for next row calculation

nextSpriteRow:
   ex   de, hl       ;; [1] Switch DE <=> HL to have HL pointing to the sprite and DE to video memory
widthRestore = .+1
   ld    b, #00      ;; [2] Restore width value before next_row loop start (00 is a placeholder)
   
nextByte:
   ld     a ,(de)    ;; [2] Get next background byte into A
   and  (hl)         ;; [2] Erase background part that is to be overwritten (Mask step 1)
   inc   hl          ;; [2] HL++ => Point HL to Sprite Colour information
   or   (hl)         ;; [2] Add up background and sprite information in one byte (Mask step 2)
   ld   (de), a      ;; [2] Save modified background + sprite data information into memory
   inc   de          ;; [2] DE++ | 
   inc   hl          ;; [2] HL++ |> Next bytes (sprite and memory)

   djnz  nextByte    ;; [3/4] B--; While not 0, we are still painting this sprite row, so continue with next Byte

   dec__ixh          ;; [2] IXH--; 1 more row of the sprite completed
   jr     z, return  ;; [2/3] If that was the last row, we safely return

   ;; Jump destination pointer to the start of the next row in video memory
   ex    de, hl      ;; [1] Switch destination (DE) and source (HL) to have destination into HL for 16bit maths
minusWidthOp = .+1
   ld    bc, #0xF700 ;; [3] BC = -(0x800 + sprite width) => Offset to jump to previous row (00 is a placeholder)
   add   hl, bc      ;; [3] We substract 0x800 plus the width of the sprite (BC) to destination pointer 

   ld     a, h       ;; [1] We check if we have crossed video memory boundaries (which will happen every 8 rows). 
                     ;; .... If that happens, bits 13,12 and 11 of destination pointer will be 1
   and   #0x38       ;; | [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   xor   #0x38       ;; | [2] ... inverted (so that they 3 turn to 0 if they were 1)
   jr   nz, nextSpriteRow ;; [2/3] If any bit from {13,12,11} is not 0, we are still inside 
                          ;; .... video memory boundaries, so proceed with next row

   ;; Every 8 rows, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means we have to advance 16K 
   ;; to return to the video memory and then jump -0x50 back to move to 
   ;; previous row. So, in a single operation, we need to do 
   ;; DE += 0x4000 - 0x50 => DE += 0x3FB0
   ld    bc, #0x3FB0    ;; [3] We advance destination pointer to next row
   add   hl, bc         ;; [3]  HL -= 0xC050 (+=3FB0)
   jr    nextSpriteRow  ;; [3] Continue copying

return:
   ;; Ret instruction is provided by C/ASM bindings