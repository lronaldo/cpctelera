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
;; Function: cpct_drawSpriteVFlip_f
;;
;;    Copies a sprite from an array to video memory or Hardware Back Buffer 
;; flipping it vertically (top to bottom)
;;
;; C Definition:
;;    void <cpct_drawSpriteVFlip_f> (void* *sprite*, void* *memory*, <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B HL) sprite - Source Sprite Pointer
;;  (2B DE) memory - Destination video memory pointer (*Bottom-left corner*)
;;  (1B C ) width  - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (1B B ) height - Sprite Height in bytes 
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteVFlip_f_asm
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
;;  * *width* (1-63) must be the width of the sprite *in bytes*. Always remember 
;; that the width must be  expressed in bytes and *not* in pixels. 
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;
;; Known limitations:
;;    * A sprite *width* outside the range (1-63) will probably make the program 
;; hang or crash, due to the optimization technique used (unfolded LDI loop).
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
;;    As the function uses an unrolled LDI loop containing 63 LDI's, the maximum 
;; with of any *sprite* to be drawn is 63. If you try to draw a wider sprite,
;; the function behaviour will be undefined.
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
;;       cpct_drawSpriteVFlip_f(o->sprite, pvmem, o->width, o->height);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 167 bytes
;;   ASM-bindings - 162 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;  Best      | 14 + (24 + 5W)H + 9HH  | 56 + (96 + 20W)H + 36HH
;;  Worst     |       Best + 9         |      Best + 36
;; ----------------------------------------------------------------
;;  W=2,H=16  |        558 /  567      |   2232 / 2268
;;  W=4,H=32  |       1422 / 1431      |   5688 / 5724
;; ----------------------------------------------------------------
;; Asm saving |         -16            |        -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = int((H-1)/8)
;;
;; Credits:
;;    This routine was inspired in the original *cpc_PutSprite* from
;; CPCRSLib by Raul Simarro.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Modify code using width to jump in drawSpriteWidth
   ld    a, #126           ;; [2] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite * 2 (2B)
   sub   c                 ;; [1]    to do as much LDIs as bytes the Sprite is wide
   sub   c                 ;; [1]
   ld  (ds_jrOffset), a    ;; [4] Modify JR offset to create the jump we need

   ld    a, b              ;; [1] A = Height (used as counter for the number of lines we have to copy)
   ex   de, hl             ;; [1] Instead of jumping over the next line, we do the inverse operation because 
                           ;; .... it is only 4 cycles and not 10, as a JP would be)

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   ex   de, hl             ;; [1] HL and DE are exchanged every line to do 16bit maths with DE. 
                           ;; .... This line reverses it before proceeding to copy the next line.
ds_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   ld   bc, #-0x800 ;; [3] 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines
                    ;; ... Each LDI performed will decrease this by 1, as we progress through memory copying the present line
ds_jrOffset = .+1   ;;
   jr__0            ;; [3] Self modifying instruction: the '00' will be substituted by the required jump forward. 
                    ;; ... (Note: Writting JR 0 compiles but later it gives odd linking errors)
   ldi              ;; [5] <| 63 LDIs, which are able to copy up to 63 bytes each time.
   ldi              ;; [5]  | That means that each Sprite line should be 63 bytes width at most.
   ldi              ;; [5]  | The JR instruction at the start makes us ignore the LDIs we don't need 
   ldi              ;; [5] <| (jumping over them) That ensures we will be doing only as much LDIs 
   ldi              ;; [5] <| as bytes our sprite is wide.
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
 
   dec   a          ;; [1] Another line finished: we discount it from A
   ret   z          ;; [2/4] If that was the last line, we safely return

   ;; Jump destination pointer to the start of the next line in video memory
   ex   de, hl      ;; [1] DE has destination, but we have to exchange it with HL to be able to do 16bit maths
   add  hl, bc      ;; [3] We substract 0x800 plus the width of the sprite (BC) to destination pointer 
   ld    b, a       ;; [1] Save A into B (B = A)
   ld    a, h       ;; [1] We check if we have crossed video memory boundaries (which will happen every 8 lines). 
                    ;; .... If that happens, bits 13,12 and 11 of destination pointer will be 1
   and   #0x38      ;; | [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   xor   #0x38      ;; | [2] ... inverted (so that they 3 turn to 0 if they were 1)
   ld    a, b       ;; [1] Restore A from B (A = B)
   jp   nz, ds_drawSpriteWidth_next ;; [3] If any bit from {13,12,11} is not 0, we are still inside 
                                    ;; .... video memory boundaries, so proceed with next line

   ;; Every 8 lines, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means our next line is 16K-0x50 bytes back
   ;; which is the same as advancing 48K+0x50 = 0xC050 bytes, as memory is 64K 
   ;; and our 16bit pointers cycle over it
   ld   bc, #0x3FB0           ;; [3] We advance destination pointer to next line
   add  hl, bc                ;; [3]  HL -= 0xC050 (+=3FB0)
   jp ds_drawSpriteWidth_next ;; [3] Continue copying
