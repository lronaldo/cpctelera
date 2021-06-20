;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_hflipSpriteM2_r
;;
;;   Horizontally flips a sprite, encoded in screen pixel format, *mode 2*
;; (ROM-friendly version).
;;
;; C definition:
;;   void <cpct_hflipSpriteM2_r> (void* sprite, <u8> width, <u8> height) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (2B DE) sprite - Pointer to the sprite array (first byte of consecutive sprite data)
;;  (1B  L) width  - Width of the sprite in *bytes* (*NOT* in pixels!). Must be >= 1.
;;  (1B  H) height - Height of the sprite in pixels / bytes (both are the same). Must be >= 1.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_hflipSpriteM2_r_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; (Mode 2 data, 8 pixels per byte). Sprite must be rectangular and all bytes in the array 
;; must be consecutive pixels, starting from top-left corner and going left-to-right, top-to-bottom 
;; down to the bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*,
;; having *width* in bytes. Each byte of the sprite encodes 8 pixles in screen pixel format mode 2 
;; (1 bit per pixel, all pixels consecutive). *Warning!* This function will admit any 16-bits value 
;; as sprite pointer and will *modify* bytes at the given address. On giving an incorrect address 
;; this function will yield undefined behaviour, probably making your program crash or doing odd 
;; things due to part of the memory being altered by this function.
;;  * *width* must be the width of the sprite *in bytes* and must be 1 or more. 
;; Using 0 as *width* parameter for this function could potentially make the program 
;; hang or crash. Always remember that the *width* must be expressed in bytes 
;; and *not* in pixels. The correspondence is 1 byte = 8 pixels (mode 2)
;;  * *height* must be the height of the sprite in pixels or bytes (both should be the
;; same amount), and must be greater than 0. There is no practical upper limit to this value,
;; but giving a height greater than the height of the sprite will yield undefined behaviour,
;; as bytes after the sprite array might result modified. 
;;
;; Known limitations:
;;  * This function does not do any kind of boundary check. If you give it 
;; incorrect values for *width*, *height* or *sprite* pointer it might 
;; potentially alter the contents of memory locations beyond *sprite* boundaries. 
;; This could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that your values are correct.
;;  * As this function receives a byte-pointer to memory, it can only 
;; flip byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte.
;;
;; Details:
;;    This function performs an horizontal flipping of the given sprite. To 
;; do so, the function inverts byte-order in all sprite rows, and also inverts
;; pixel order inside each byte of the sprite. The function expects to receive
;; an sprite in screen pixel format (mode 2), like in this example:
;;
;; (start code)
;;    // Example call. Sprite has 24x4 pixels (3x4 bytes)
;;    cpct_hflipSpriteM0(3, 4, sprite);
;;
;;    // Operation performed by the call and results
;;    //
;;    // -----------------------------------------------------------------------------------------
;;    //  |         Received as parameter            |           Result after flipping           |
;;    // -----------------------------------------------------------------------------------------
;;    //  | sprite => [01235678][9ABCDEFG][HIJKLMNO] |  sprite => [ONMLKJIH][GFEDCBA9][87653210] |
;;    //  |           [qwertyui][asdfghjk][zxcvbnmp] |            [pmnbvcxz][kjhgfdsa][iuytrewq] |
;;    //  |           [01235678][9ABCDEFG][HIJKLMNO] |            [01235678][9ABCDEFG][HIJKLMNO] |
;;    //  |           [qwertyui][asdfghjk][zxcvbnmp] |            [qwertyui][asdfghjk][zxcvbnmp] |
;;    // -----------------------------------------------------------------------------------------
;;    //  Sprite takes 12 consecutive bytes in memory (4 rows with 3 bytes, and 8 pixels 
;;    //  each byte, for a total of 24x4 pixels, 96 pixels)
;;    //
;; (end code)
;;
;;    As can be seen on the example, the function modifies the bytes of the 
;; *sprite* in-place. Therefore, the *sprite* becomes horizontally flipped
;; after the call and will not return to normal status unless another 
;; horizontally flipping operation is performed.
;;
;;    This function has a high performance if taking into account that it is not 
;; using a conversion table. However, it is very slow compared to functions that 
;; use memory-aligned conversion tables. The advantage is that this function takes
;; less memory space as it does not require the 256-bytes table to be held in 
;; memory. Therefore, the main use for this function is to save space in memory
;; whenever fast flipping is not required. If fast performance is required, it 
;; is better to consider the use of functions with memory-aligned conversion tables.
;;
;; Use example:
;;    Next example presents a function that draws all enemy ships in a game.
;; Ships are stored in an array, ordered by direction they are facing. Therefore,
;; first ships in the array are facing left, and the rest are facing right. 
;; There is also a count of the total number of ships facing left, so as not to
;; be continuously testing.
;; (start code)
;;    // Draws all the enemy ships that are visible in the present screen.
;;    // The enemy ships array contains all the ships facing left first, then
;;    // all of those facing right. 
;;    void drawEnemyShips(TEnemyShip* ships, u8 leftships, u8 rightships) {
;;       // First draw all ships that are facing left
;;       while (leftships--) {
;;          // Draw Ship
;;          u8 pvmem = cpct_getScreenPtr(CPCT_VMEM_START, ships->x, ships->y);
;;          cpct_drawSprite(g_shipSprite, pvmem, SHIP_WIDTH, SHIP_HEIGHT);
;;
;;          // Move the pointer to point to the next ship
;;          ships++;
;;       }
;;
;;       // Now proceed to flip the sprite to be able to draw ships facing right
;;       cpct_hflipSpriteM2_r(g_shipSprite, SHIP_WIDTH, SHIP_HEIGHT);
;;
;;       // Now draw all ships facing right
;;       while (rightships--) {
;;          // Draw Ship
;;          u8 pvmem = cpct_getScreenPtr(CPCT_VMEM_START, ships->x, ships->y);
;;          cpct_drawSprite(g_shipSprite, pvmem, SHIP_WIDTH, SHIP_HEIGHT);
;;
;;          // Move the pointer to point to the next ship
;;          ships++;
;;       }
;;
;;       // Flip the sprite again to left it as it was at the start
;;       cpct_hflipSpriteM2_r(g_shipSprite, SHIP_WIDTH, SHIP_HEIGHT);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 66 bytes
;;  ASM-bindings - 63 bytes
;;
;; Time Measures:
;; (start code)
;;  Case       |      microSecs (us)       |         CPU Cycles          |
;; -----------------------------------------------------------------------
;;  Even-width |     (55WW + 24)H + 14     |     (220WW +  96)H +  56    |
;;   Odd-width |     (54WW + 55)H + 14     |     (216WW + 220)H +  56    |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |          1278             |           5112              |
;;  W=5,H=32   |          5230             |          20920              |
;; -----------------------------------------------------------------------
;;  Asm saving |          -12              |            -48              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *WW* = *width*/2, *H* = *height*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; This loop is repeated for every vertical row of the sprite
;;   Set HL to point to the end of present sprite row, as DE
;;   already points to the start of it.
;;
nextrow:
   push hl      ;; [4] Save width/height into the stack for later use
   ld    b, l   ;; [1] b = width (to count how many bytes are to be reversed by inner loop)
   ld    h, #0  ;; [2] hl = l, setting hl = width to use it for math purposes 
   add  hl, de  ;; [3] hl = de + hl (start of row pointer + width, so hl points to start of next sprite row)
   push hl      ;; [4] Save pointer to the start of next sprite row
   dec  hl      ;; [2] Make hl point to the last byte of the present sprite row

   jr  first    ;; [3] Jump to start processing present sprite row

;; This loop is repeated for every horizontal byte in the same sprite row.
;; It reverses each byte's pair of colours, and it also reverses byte order
;; inside the same row. So, virtual layout scheme is as follows:
;; --------------------------------------------------------------------------
;; byte order    |      x        y               y         x
;; pixel values  | [01234567][89ABCDEF] --> [FEDCBA98][76543210]
;; --------------------------------------------------------------------------
;; byte layout   |                 
;; (Mode 2,8 px) | [ 01234567 ] --> [ 76543210 ] << Pixel bits (pixels 0 to 7)
;;
nextbyte:
   ld (de), a     ;; [2] Save last reversed byte into destination
   inc  de        ;; [2] Start-of-the-row pointer advances to next byte to be reversed 
   dec  hl        ;; [2] End-of-the-row pointer moves 1 byte backwards to the next byte to be reversed

;; DE points to the start of the byte row and increasing, whereas 
;; HL points to the end of the byte row and decreasing. 
;;    This inner part of the code reverses byte order inside the row
;;    and pixel order inside each byte
;;
first:
   ;;
   ;; This part reverses (DE) byte and places it at (HL) location, 
   ;; taking a byte from the start of the row and placing it at the end
   ;;
   ex  de, hl     ;; [1] DE <-> HL to use HL to refer to the byte going to be reversed now
   ld   a, (hl)   ;; [2]  A=next byte to be reversed
   ld   c, a      ;; [1]  A=C=pixels to be reversed, as required by revert macro

   cpctm_reverse_mode_2_pixels_of_A c ;; [16] Reverse the bits of A (using C as temporary storage)

   dec  b         ;; [1] B-- (One less byte to be reversed)
   jr   z, end    ;; [2/3] If B=0, this was the last byte to be reversed, son got to end

   ;;
   ;; This part reverses (DE) byte and places it at (HL) location too,
   ;; but taking bytes from the end of the row and placing them at the start
   ;;
   ex  de, hl     ;; [1] DE <-> HL to use HL to refer to the byte going to be reversed now
   ld   c, (hl)   ;; [2] C=Next byte to be reversed
   ld (hl), a     ;; [2] Save previously reverted byte
   ld   a, c      ;; [1] A=C=pixels to be reversed, as required by revert macro
   
   cpctm_reverse_mode_2_pixels_of_A c ;; [16] Reverse the bits of A (using C as temporary storage)

   djnz nextbyte  ;; [3/4] B--, if B!=0, continue reversing next byte

;; Finished reversing present byte row from the sprite
;; 
end:
   ld (de), a     ;; [2] Save last reversed byte into its final destination

   pop  de        ;; [3] DE points to the start of next sprite byte row (saved previously)
   pop  hl        ;; [3] HL contains (H: height, L: width of the sprite)

   dec   h        ;; [1] H--, One less sprite row to finish
   jr   nz, nextrow ;; [2/3] If H!=0, process next sprite row, as there are more

   ret            ;; [3] All sprite rows reversed. Return.