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
;; Function: cpct_drawSpriteVFlip
;;
;;    Copies a sprite from an array to video memory or Hardware Back Buffer 
;; flipping it vertically (top to bottom)
;;
;; C Definition:
;;    void <cpct_drawSpriteVFlip> (void* *sprite*, void* *memory*, <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B DE) sprite - Source Sprite Pointer
;;  (2B HL) memory - Destination video memory pointer (*Bottom-left corner*)
;;  (1B A ) width  - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (1B B ) height - Sprite Height in bytes 
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteVFlip_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be a pointer to the sprite array containing pixel data to
;; be drawn. Sprite must be rectangular and all bytes in the array must be 
;; consecutive pixels, starting from top-left corner and going left-to-right, 
;; top-to-bottom down to the bottom-right corner. Total amount of bytes in pixel 
;; array should be *width* x *height*.
;;  * *memory* must be a pointer to bottom-left corner of the place where the sprite 
;; is to be drawn. The sprite will then be drawn bottom-to-top, starting at the 
;; video memory byte *memory* is pointing at. It could be any place in memory, 
;; inside or outside current video memory. It will be equally treated as video 
;; memory (taking into account CPC's video memory disposition). This lets you 
;; copy sprites to software or hardware backbuffers, and not only video memory.
;;  * *width* (1-255) must be the width of the sprite *in bytes*. Always remember 
;; that the width must be  expressed in bytes and *not* in pixels. 
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;
;; Known limitations:
;;    * A sprite *width* of 0 will be considered as 65536, effectively overwritting
;; your whole memory and making your program crash.
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
;;    Draws given *sprite* to *memory* taking into account CPC's standard video
;; memory disposition. Therefore, *memory* can be pointing to actual video memory
;; or to a hardware backbuffer. 
;;
;;    The drawing happens bottom-to-top. *memory* will be considered to be pointing
;; to the bottom-left byte of the sprite in video memory. This is the way in which
;; the sprite will be drawn in reverse,
;;
;; (start code)
;; ||-----------------------||----------------------||
;; ||   MEMORY              ||  VIDEO MEMORY        ||
;; ||-----------------------||----------------------|| 
;; ||           width       ||          width       ||
;; ||         (------)      ||        (------)      ||
;; ||                       ||                      ||
;; ||      /->###  ###  ^   ||        # #  # #  ^   || ^
;; || sprite  ##    ##  | h ||        ##    ##  | h || |
;; ||         #      #  | e ||        ###  ###  | e || | Sprite is 
;; ||         ###  ###  | i ||        ##   ###  | i || | drawn in 
;; ||         ##   ###  | g ||        ###  ###  | g || | this order
;; ||         ###  ###  | h ||        #      #  | h || | (bottom to
;; ||         ##    ##  | t || memory ##    ##  | t || |  top)
;; ||         # #  # #  v   ||     \->###  ###  v   || -  
;; ||-----------------------||----------------------||
;; (end code)
;;
;;    In order to get a pointer to the bottom-left of the memory location where you
;; want to draw your sprite, you may use <cpct_getBottomLeftPtr>.
;;
;;    Use example,
;; (start code)
;;    ///////////////////////////////////////////////////////////////////
;;    // DRAW OBJECT IN FRONT OF INVERTING MIRROR
;;    //    Draws an object in its coordinates and a vertically inverted
;;    // version of the same object right next to the original one.
;;    //
;;    void drawObjectInFrontOfMirror(Object* o) {
;;       u8* pvmem;  // Pointer to video memory
;;    
;;       //-----Draw original object
;;       //
;;       // Get a pointer to video memory byte for object location
;;       pvmem = cpct_getScreenPtr(CPCT_VMEM_START, o->x, o->y);
;;       // Draw the object
;;       cpct_drawSprite(o->sprite, pvmem, o->width, o->height);
;;    
;;       //-----Draw Inverted object right next to original one
;;       //
;;       // Assuming pvmem points to upper-left byte of the original sprite in
;;       // video memory, calculate a pointer to the bottom-left byte (in video memory).
;;       // Equivalent to: cpct_getScreenPtr(CPCT_VMEM_START, o->x, (o->y + o->height - 1) )
;;       pvmem = cpct_getBottomLeftPtr(pvmem, o->height);
;;       // As we don't want to overwrite the original object, this inverted version will
;;       // be drawn 1 byte to its right (in front of the original). That means moving to 
;;       // the right (adding) a number of bytes equal to the width of the object + 1. 
;;       pvmem += o->width + 1;
;;       // Finally, draw the vertically flipped object. This draw function
;;       // does the drawing bottom-to-top in the video memory. That's the reason
;;       // to have a pointer to the bottom-left.
;;       cpct_drawSpriteVFlip(o->sprite, pvmem, o->width, o->height);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 43 bytes
;;   ASM-bindings - 37 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;  Best      | 13 + (23 + 6W)H + 8HH  | 52 + (92 + 24W)H + 32HH
;;  Worst     |       Best + 8         |      Best + 32
;; ----------------------------------------------------------------
;;  W=2,H=16  |        573 /  581      |   2292 / 2324
;;  W=4,H=32  |       1549 / 1557      |   6196 / 6228
;; ----------------------------------------------------------------
;; Asm saving |         -17            |        -68
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = int((H-1)/8)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Modify code using width to jump in drawSpriteWidth
   ld (widthRestore), a ;; [4] Save sprite width into its restoration placeholder for row loop
   neg                  ;; [2] A = -A (0 - Sprite Width)
   ld (minusWidthOp), a ;; [4] Save negative sprite width into placeholder for next row calculation
   ld     a, b          ;; [1] A = height

   ;; Draw the sprite row by row
nextSpriteRow:
   ex   de, hl       ;; [1] Put destination into DE and source into HL for copying with LDIR
widthRestore = .+1
   ld   bc, #0x0000  ;; [3] B=0, C=Sprite Width. This value is a placeholder that gets modified 
   ldir              ;; [6W-1] Draw a complete sprite row byte by byte
 
   dec   a           ;; [1] Another row finished: we discount it from A
   ret   z           ;; [2/4] If that was the last row, we safely return

   ;; Jump destination pointer to the start of the next row in video memory
   ex   de, hl       ;; [1] Switch destination (DE) and source (HL) to have destination into HL for 16bit maths
minusWidthOp = .+1
   ld   bc, #0xF700  ;; [3] BC = -(0x800 + sprite width) => Offset to jump to previous row (00 is a placeholder)
   add  hl, bc       ;; [3] We substract 0x800 plus the width of the sprite (BC) to destination pointer 

   ld    b, a        ;; [1] Save A into B (B = A)
   ld    a, h        ;; [1] We check if we have crossed video memory boundaries (which will happen every 8 rows). 
                     ;; .... If that happens, bits 13,12 and 11 of destination pointer will be 1
   and   #0x38       ;; | [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   xor   #0x38       ;; | [2] ... inverted (so that they 3 turn to 0 if they were 1)
   ld    a, b        ;; [1] Restore A from B (A = B)
   jr   nz, nextSpriteRow ;; [2/3] If any bit from {13,12,11} is not 0, we are still inside 
                          ;; .... video memory boundaries, so proceed with next row

   ;; Every 8 rows, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means we have to advance 16K 
   ;; to return to the video memory and then jump -0x50 back to move to 
   ;; previous row. So, in a single operation, we need to do 
   ;; DE += 0x4000 - 0x50 => DE += 0x3FB0
   ld   bc, #0x3FB0  ;; [3] We advance destination pointer to next row
   add  hl, bc       ;; [3]  HL -= 0xC050 (+=3FB0)
   jr nextSpriteRow  ;; [3] Continue copying