;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_mirrorSpriteM0
;;
;;   Mirrors a *mode 0* encoded sprite left-to-right and vice-versa.
;;
;; C definition:
;;   void <cpct_mirrorSpriteM0> (<u8>*sprite, <u8> width, <u8> height);
;;
;; Input Parameters (4 bytes):
;;  (2B DE) sprite - Pointer to the sprite array (first byte of consecutive sprite data)
;;  (1B  H) width  - Width of the sprite in *bytes* (*NOT* in pixels!). Must be >= 1.
;;  (1B  L) height - Height of the sprite in pixels / bytes (both are the same). Must be >= 1.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_mirrorSpriteM0_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; (Mode 0 data). Sprite must be rectangular and all bytes in the array must be consecutive 
;; pixels, starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>). *Warning!* This function
;; will admit any 16-bits value as sprite pointer and will *modify* bytes at the given
;; address. On giving an incorrect address this function will yield undefined behaviour,
;; probably making your program crash or doing odd things due to part of the memory being
;; altered by this function.
;;  * *width* must be the width of the sprite *in bytes* and must be 1 or more. 
;; Using 0 as *width* parameter for this function could potentially make the program 
;; hang or crash. Always remember that the *width* must be expressed in bytes 
;; and *not* in pixels. The correspondence is 1 byte = 2 pixels (mode 0)
;;  * *height* must be the height of the sprite in pixels / bytes (both should be the
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
;; mirror byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte.
;;
;; Details:
;;    This function performs an horizontal mirroring of the given sprite. To 
;; do so, the function inverts byte-order in all sprite rows, and also inverts
;; pixel order inside each byte of the sprite. The function expects to receive
;; an sprite in screen pixel format (mode 0), like in this example:
;;
;; (start code)
;;    // Example call. Sprite has 8x4 pixels (4x4 bytes)
;;    cpct_mirrorSpriteM0(sprite, 4, 4);
;;
;;    // Operation performed by the call and results
;;    //
;;    // --------------------------------------------------------------
;;    //  |  Received as parameter     | Result after mirroring      |
;;    // --------------------------------------------------------------
;;    //  | sprite => [05][21][73][40] |  sprite => [04][37][12][05] |
;;    //  |           [52][23][37][74] |            [47][73][32][25] |
;;    //  |           [05][11][31][04] |            [04][13][11][50] |
;;    //  |           [00][55][44][00] |            [00][44][55][00] |
;;    // --------------------------------------------------------------
;;    //  Sprite takes 16 consecutive bytes in memory (4 rows with 4 bytes)
;;    //
;; (end code)
;;
;;    As can be seen on the example, the function modifies the bytes of the 
;; *sprite* in-place. Therefore, the *sprite* becomes horizontally mirrored
;; after the call and will not return to normal status unless another 
;; horizontally mirroring operation is performed.
;;
;;    This function has a high performance if taking into account that it is not 
;; using a conversion table. However, it is very slow compared to functions that 
;; use memory-aligned conversion tables. The advantage is that this function takes
;; less memory space as it does not require the 256-bytes table to be held in 
;; memory. Therefore, the main use for this function is to save space in memory
;; whenever fast mirroring is not required. If fast performance is required, it 
;; is better to consider the use of functions with memory-aligned conversion tables.
;;
;; Use example:
;;    Next example shows how to create a function for drawing a 2D-Character
;; sprite that can look either to the left or to the right. Sprite is only 
;; mirrored when the character changes the side it is looking.
;; (start code)
;;    // Draws the main character sprite always looking to the 
;;    // appropriate side (right or left), reversing it whenever required
;;    void drawCharacter(u8 lookingat, u8 x, u8 y) {
;;       u8* pvmem; // Pointer to video memory to draw the sprite
;;
;;       // Check if we have to reverse character sprite or not
;;       if(lookingAt != wasLookingAt) {
;;          // Horizontally mirror character sprite when it 
;;          // changes the side it is looking at
;;          cpct_mirrorSpriteM0(characterSprite, 4, 8);
;;          wasLookingAt = lookingAt;
;;       }
;;
;;       // Draw main character's sprite
;;       pvmem = cpct_getScreenPtr(CPCT_VMEM, x, y);
;;       cpct_drawSprite(characterSprite, pvmem, 4, 8);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - ?? bytes
;;  ASM-bindings - ?? bytes
;;
;; Time Measures:
;; (start code)
;;  Case       |      microSecs (us)       |         CPU Cycles          |
;; -----------------------------------------------------------------------
;;  Best       |  24H + (38WW + 16W)H + 14 |  96H + (152WW + 64W)H + 56  |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |        
;;  W=4,H=32   |       
;; -----------------------------------------------------------------------
;;  Asm saving |          -12              |            -48              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *W* = *width* % 2, *WW* = *width*/2, *H* = *height*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Parameter retrieval
   pop  hl     ;; [3] HL = return address
   pop  de     ;; [3] DE = Sprite start address pointer
   ex (sp), hl ;; [6] HL = height / width, while leaving return address in the
               ;; ... stack, as this function uses __z88dk_callee convention

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
;; byte order    |   A   B   C   D       D   C   B   A
;; pixel values  | [12][34][56][78] --> [87][65][43][21]
;; --------------------------------------------------------------------------
;; byte layout   | [ 0123 4567 ]     [ 1032 5476 ] << Bit order (reversed by pairs)
;; (Mode 0,2 px) | [ 0101 0101 ] --> [ 1010 1010 ] << Pixel bits (pixel 1, pixel 0)
;;
nextbyte:
   ld (de), a     ;; [2] Save last reversed byte into destination
   inc  hl        ;; [2] Start-of-the-row pointer advances to next byte to be reversed 
   dec  de        ;; [2] End-of-the-row pointer moves 1 byte backwards to the next byte to be reversed

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

   ld   a, (hl)    ;; [2] A=byte to be reversed	
   ld   c, a       ;; [1] C=A (Copy to be used later for mixing bits)
   rlca            ;; [1] | Rotate A twice to the left to get bits ordered...
   rlca            ;; [1] | ... in the way we need for mixing, A=[23456701]
  
   ;; Mix C with A to get pixels reversed by reordering bits
   xor  c          ;; [1] |  C = [01234567]
   and #0b10101010 ;; [2] |  A = [23456701]
   xor  c          ;; [1] | A2 = [03254761]
   rrca            ;; [1] Rotate right to get pixels reversed A = [10325476]

   dec  b          ;; [1] B-- (One less byte to be reversed)
   jr   z, end     ;; [2/3] If B=0, this was the last byte to be reversed, son got to end

   ;;
   ;; This part reverses (DE) byte and places it at (HL) location too,
   ;; but taking bytes from the end of the row and placing them at the start
   ;;
   ex  de, hl      ;; [1] DE <-> HL to use HL to refer to the byte going to be reversed now
   ld   c, (hl)    ;; [2] C=Next byte to be reversed
   ld (hl), a      ;; [2] Save previously reversed byte into its destination 

   ld   a, c       ;; [1] A=Next byte to be reversed
   rlca            ;; [1] | Rotate A twice to the left to get bits ordered...
   rlca            ;; [1] | ... in the way we need for mixing, A=[23456701]
  
   ;; Mix C with A to get pixels reversed by reordering bits
   xor  c          ;; [1] |  C = [01234567]
   and #0b10101010 ;; [2] |  A = [23456701]
   xor  c          ;; [1] | A2 = [03254761]
   rrca            ;; [1] Rotate right to get pixels reversed A = [10325476]

   djnz next       ;; [3/4] B--, if B!=0, continue reversing next byte

;; Finished reversing present byte row from the sprite
;; 
end:
   ld (de), a       ;; [2] Save last reversed byte into its final destination

   pop  de          ;; [3] DE points to the start of next sprite byte row (saved previously)
   pop  hl          ;; [3] HL contains (H: height, L: width of the sprite)

   dec   h          ;; [1] H--, One less sprite row to finish
   jr   nz, nextrow ;; [2/3] If H!=0, process next sprite row, as there are more

   ret              ;; [3] All sprite rows reversed. Return.
