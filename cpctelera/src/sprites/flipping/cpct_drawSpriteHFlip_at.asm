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
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawSpriteHFlip_at
;;
;;    Draws a Mode 0 sprite from an array to video memory or Hardware Back Buffer 
;; flipping it Horizontally (right to left)
;;
;; C Definition:
;;    void <cpct_drawSpriteHFlip_at> (const void* *sprite*, void* *memory*, 
;;                            <u8> *width*, <u8> *height*, const void* *pfliptable*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;  (2B  HL) sprite     - Source Sprite Pointer
;;  (2B  DE) memory     - Destination video memory pointer (top-left byte)
;;  (1B IXL) width      - Sprite Width in *bytes* (Beware, *not* in pixels!)
;;  (1B IXH) height     - Sprite Height in bytes 
;;  (1B  A ) pfliptable - Most Significant Byte of 256-byte flip table address (LSB should be 00)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteHFlip_at_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be a pointer to the sprite array containing Mode 0 pixel data 
;; to be drawn. Sprite must be rectangular and all bytes in the array must be 
;; consecutive pixels, starting from top-left corner and going left-to-right, 
;; top-to-bottom down to the bottom-right corner. Total amount of bytes in pixel 
;; array should be *width* x *height*.
;;  * *memory* must be a pointer to the place where the sprite is to be drawn.
;; It must be pointing to the top-left byte in video memory where the sprite is
;; going to be drawn. From this pointer, the top-right byte will be calculated adding
;; *width* - 1, as the drawing is performed right-to-left instead of left-to-right. 
;; It could be any place in memory, inside or outside current video memory. It will 
;; be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, 
;; and not only video memory.
;;  * *width* (1-256) must be the width of the sprite *in bytes*. Always remember 
;; that the width must be expressed in bytes and *not* in pixels. 
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;  * *pfliptable* must be a pointer to the address where the horizontally flipping
;; table starts. This table must by *256-byte aligned* and only Most Significant Byte (MSB)
;; of the address will be used. Least Significant Byte (LSB) will be set to 0, 
;; no matter what address is passed.
;;
;; Known limitations:
;;    * *width*s or *height*s of 0 will be considered as 256, and could 
;; potentially make your code behave erratically or crash.
;;    * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;    * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. 
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Details:
;;    Draws given *sprite* to *memory* taking into account CPC's standard video
;; memory disposition. Drawing is performed right-to-left instead of normal way
;; (left-to-right). This effectively produces an horizontally flipped version
;; of the sprite in video memory.
;;
;; See next figure for details,
;; (start code)
;; ||-----------------------||-----------------------||
;; ||   MEMORY              ||  VIDEO MEMORY         ||
;; ||-----------------------||-----------------------|| 
;; ||           width       ||           width       ||
;; ||         (------)      ||         (------)      ||
;; ||                       ||                       ||
;; ||      /->########  ^   ||      /->########  ^   ||
;; || sprite  #      #  | h || memory  #      #  | h ||
;; ||         #  #####  | e ||         #####  #  | e ||
;; ||         #     ##  | i ||         ##     #  | i ||
;; ||         #    ###  | g ||         ###    #  | g ||
;; ||         #  #####  | h ||         #####  #  | h ||
;; ||         #      #  | t ||         #      #  | t ||
;; ||         ########  v   ||         ########  v   ||
;; ||-----------------------||-----------------------||
;;                                    <--------  Sprite is drawn in this order
;;                                    right-to-left and top-to-bottom
;; (end code)
;;
;;    The function uses a 256-byte aligned table to flip pixels inside each byte.
;; This table should have been defined previously (you may use <cpctm_createPixelFlippingTable>
;; for that). This function works for all 3 videomodes (Mode 0, 1 or 2). However, 
;; pixel flipping is mode-dependent and different mode require different flipping
;; tables. So, depending on the flipping table you use, flipping will be performed
;; for mode 0, 1 or 2.
;;
;;    Use example,
;; (start code)
;;    // Declare a 256-byte aligned pixel flipping table, for Mode 0, 
;;    // that has been defined in another translation file using 
;;    // cpctm_createPixelFlippingTable(g_pflipTable, 0x0200, M0);
;;    // This lets us use the table without redefinining it (we only need one)
;;    cpctm_declarePixelFlippingTable(g_pflipTable);
;;
;;    ///////////////////////////////////////////////////////////////////
;;    // DRAW ENTITY LOOKING LEFT OR RIGHT
;;    //    Draws an entity looking to either side. The original sprite
;;    // is assumed to be looking to the right, so we only need to draw 
;;    // it horizontally flipped when we want it to look left. 
;;    //
;;    void drawEntityLookingAt(Entity* e, void* p_videomem, TSide look_at) {
;;       // Check how to draw the entity depending on where it is looking at
;;       if (look_at == RIGHT) {
;;          // Entity is looking right, just draw it the normal way
;;          cpct_drawSprite(e->sprite, p_videomem, o->width, o->height);
;;       } else {
;;          // Entity is looking left. We must draw it horizontally flipped
;;          cpct_drawSpriteHFlip_at(o->sprite, p_videomem, o->width, o->height, g_pfliptable);
;;       }
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;     C-bindings - AF, BC, DE, HL
;;   ASM-bindings - AF, BC, DE, HL, IX
;;
;; Required memory:
;;     C-bindings - 71 bytes
;;   ASM-bindings - 55 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)        |        CPU Cycles
;; -----------------------------------------------------------------
;;  Best      | 35 + (22 + 15W)H + 8HH  | 140 + (88 + 60W)H + 32HH
;;  Worst     |       Best + 8          |      Best + 32
;; -----------------------------------------------------------------
;;  W=2,H=16  |        875 /  883       |   3500 /  3532
;;  W=4,H=32  |       2683 / 2691       |  10732 / 10764
;; -----------------------------------------------------------------
;; Asm saving |         -31             |        -124
;; -----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = int((H-1)/8)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Set Up placeholder for flipping table
   ld (tablePage), a    ;; [4] Set Up Most Significant Byte of Flipping table to be restored before drawing each new row

   ;; Set Up placeholders to restore sprite width for each new row
   ld__a_ixl            ;; [2] A = Width
   ld (widthRestore), a ;; [4] Set up Width restoration value in C register for each new sprite row
   ld (addWidth), a     ;; [4] Set up Width to be added to HL for next video memory row to be filled up

   ;; Make DE point to the right-most byte of the first row of the screen to be drawn
   dec    a             ;; [1] A = width - 1
   add_de_a             ;; [5] DE += width - 1 (Point to last byte at the screen)

   jr  firstbyte        ;; [3] First byte does not require C to be restored nor DE/HL to be changed

nextrow:
   ex    de, hl         ;; [1] Interchange again DE<=>HL to have them in their normal configuration
tablePage = .+1         ;; ... HL=Source Sprite, DE=Video Memory location
   ld     b, #00        ;; [2] BC points to conversion table (Restore B value. 00 is a placeholder)
                        ;; ... C is irrelevant because it is changed for each new value to be converted
nextbyte:
   dec   de             ;; [2] DE-- (Previous byte in video memory. We draw right-to-left)
   inc   hl             ;; [2] HL++ (Next sprite byte)
firstbyte:
   ld     c, (hl)       ;; [2] A = Next sprite byte
   ld     a, (bc)       ;; [2] A = Byte with pixels horizontally flipped (got from precalculated table)
   ld  (de), a          ;; [2] Save Sprite byte with both pixels reversed

   dec__ixl             ;; [2] IXL-- (One less byte in this row to go)
   jr    nz, nextbyte   ;; [2/3] If C!=0, there are more bytes, so continue with nextbyte

   ;; Finished drawing present sprite row to screen. Count 1 less row
   dec__ixh             ;; [2]   IXH-- (One less sprite row to go)
   jr     z, return     ;; [2/3] if IXL==0, all rows are finished, then return

   ;; Set Up everything for Next Sprite Row

widthRestore = .+2
   ld__ixl #00          ;; [3] Restore Width into IXL before looping over next row bytes one by one

   ;; Make HL Point to the right-most byte of next screen row to be drawn
   ex    de, hl         ;; [1] Interchange DE<=>HL to have Destination pointer in HL for doing 16-bit maths
addWidth = .+1
   ld    bc, #0x0800    ;; [3] BC = 0x800 + width  (Offset from now to right-most byte of next row)
   add   hl, bc         ;; [3] HL += 0x800 + width (HL points to right-most byte of next row)
 
   ;; Check if HL has moved outside of video memory boundaries
   ld     a, h          ;; [1]   Bits 13,12,11 of H become 0 at the same time when moving outside video memory boundaries
   and   #0x38          ;; [2]   Isolate bits 13,12,11 
   jr    nz, nextrow    ;; [2/3] If A!=0, someone of the 3 bits is not 0, so boundaries have not been crossed

   ;; HL points outside video memory area (last +0x800 crossed boundaries)   
   ;; To wrap up to the start of video memory again, add up +0xC050
   ;; (0xC000 = 48K which means jumping 3 memory banks forward, returning to original memory bank
   ;;  and 0x50 is offset for the start of next pixel line inside next character line)
   ld    bc, #0xC050    ;; [3] BC = 0xC000 + 0x50
   add   hl, bc         ;; [3] HL += 0xC050 (Wrap up HL to make it point again to previous memory bank +0x50)
   jr    nextrow        ;; [3] Continue with next sprite row
return:
   ;; Ret instruction provided by C/ASM bindings