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
;; Function: cpct_hflipSpriteMaskedM0
;;
;;   Horizontally flips a sprite, encoded in screen pixel format, *mode 0*.
;;
;; C definition:
;;   void <cpct_hflipSpriteMaskedM0> (<u8> width, <u8> height, <void>*sprite) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (1B  C) width  - Width of the sprite in *bytes* (*NOT* in pixels!). Must be >= 1.
;;  (1B  B) height - Height of the sprite in pixels / bytes (both are the same). Must be >= 1.
;;  (2B HL) sprite - Pointer to the sprite array (first byte of consecutive sprite data)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_hflipSpriteMaskedM0_asm
;;
;; Parameter Restrictions:
;;
;; *** REVIEW ***
;;
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
;;
;; *** REVIEW ***
;;
;;  * This function will not work from ROM, as it uses self-modifying code.
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
;; an sprite in screen pixel format (mode 0), like in this example:
;;
;; (start code)
;;    // Example call. Sprite has 8x4 pixels (4x4 bytes)
;;    cpct_hflipSpriteM0Masked(4, 4, sprite);
;;
;;    // Operation performed by the call and results
;;    //
;;    // ---------------------------------------------------------------------------------------------
;;    //  |          Received as parameter             |          Result after flipping              |
;;    // ---------------------------------------------------------------------------------------------
;;    //  | sprite => [05][mM][21][nN][73][mM][40][nN] |  sprite => [04][Nn][37][Mm][12][Nn][05][Mm] |
;;    //  |           [52][mM][23][nN][37][mM][74][nN] |            [47][Nn][73][Mm][32][Nn][25][Mm] |
;;    //  |           [05][mM][11][nN][31][mM][04][nN] |            [04][Nn][13][Mm][11][Nn][50][Mm] |
;;    //  |           [00][mM][55][nN][44][mM][00][nN] |            [00][Nn][44][Mm][55][Nn][00][Mm] |
;;    // ---------------------------------------------------------------------------------------------
;;    //  Sprite takes 32 consecutive bytes in memory: 4 rows with 8 bytes (4 bytes pixel data, 
;;    //  and 4 bytes mask data)
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
;;    Next example shows how to create a function for drawing a 2D-Character
;; sprite that can look either to the left or to the right. Sprite is only 
;; flipped when the character changes the side it is looking.
;; (start code)
;;    // Draws the main character sprite always looking to the 
;;    // appropriate side (right or left), reversing it whenever required
;;    void drawCharacter(u8 lookingat, u8 x, u8 y) {
;;       u8* pvmem; // Pointer to video memory to draw the sprite
;;
;;       // Check if we have to reverse character sprite or not
;;       if(lookingAt != wasLookingAt) {
;;          // Horizontally flip character's sprite when it 
;;          // changes the side it is looking at
;;          cpct_hflipSpriteM0Masked(4, 8, characterSprite);
;;          wasLookingAt = lookingAt;
;;       }
;;
;;       // Draw main character's sprite
;;       pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
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
;;  Even-width |     (32WW + 16)H + 32     |     (128WW +  64)H + 128    |
;;  Oven-width |     (32WW + 36)H + 37     |     (128WW + 144)H + 148    |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |           800             |           3200              |
;;  W=5,H=32   |          3237             |          12948              |
;; -----------------------------------------------------------------------
;;  Asm saving |          -12              |            -48              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *W* = *width* % 2, *WW* = *width*/2, *H* = *height*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  
   ;; We need HL to point to the byte previous to the start of the sprite array.
   ;; This is because HL will point to the previous byte of the next row in the 
   ;; array at the end of every loop, and hence we can add always the same quantity
   ;; (half sprite width, with +1 for odd-width sprites) to point to the centre of next row.
   dec hl       ;; [2] HL points to the byte previous to the start of the sprite array

;;
;; EXTERNAL LOOP THAT FLIPS THE SPRITE
;;    Iterates throw all the rows of the sprite
;;    
;;    This external loop starts with HL and DE pointing to the central byte/s of the row
;; and goes decreasing DE and increasing HL. At the end, DE points to the first byte of 
;; the row and HL to the last one, so HL points to the previous byte of the next row.
;; It reverses each byte's pair of colours, and it also reverses byte order inside the 
;; same row. So, virtual layout scheme is as follows:
;; --------------------------------------------------------------------------
;; byte order    |   A   B   C   D       D   C   B   A
;; pixel values  | [12][34][56][78] --> [87][65][43][21]
;; --------------------------------------------------------------------------
;; byte layout   | [ 0123 4567 ]     [ 1032 5476 ] << Bit order (reversed by pairs)
;; (Mode 0,2 px) | [ 0101 0101 ] --> [ 1010 1010 ] << Pixel bits (pixel 1, pixel 0)
;;
nextrow:
   push bc      ;; [4] Save Height/Width into the stack for later recovery
   ld   b, #0   ;; [2] BC  = C = Width (In order to easily add the width to HL)
   add  hl, bc  ;; [3] HL += BC (Add Width to start-of-row pointer, HL, to make it point to the central byte)
   ld   d, h    ;; [1] | DE = HL, so that DE also points to the central byte of the row
   ld   e, l    ;; [1] | 
   srl  c       ;; [2] C = C / 2 (Sprite width / 2)

varjump:
   ;; Even sprites jump from here directly to firstpair
   jr  nc, firstpair        ;; [3]

   ;; Odd sprites first switch the central byte, then start with the rest
   call switch_bytes_single ;; [5+13]
   inc  hl                  ;; [2]
   inc  de                  ;; [2]
   call switch_bytes_single ;; [5+13]

   ;;
   ;; INTERNAL LOOP THAT FLIPS THE SPRITE
   ;;    Iterates throw all the bytes inside each row
   ;;
   ;;    If takes byte 2-by-2, one pointed by DE and the other pointed by HL. It flips
   ;; pixels inside each byte and then switches both bytes pointed by DE and HL. There is
   ;; a special case for the central byte, at the label "one", which only flips pixels
   ;; in that byte and leaves it at the same place.
   ;;
nextpair:
   dec de          ;; [2]
   dec de          ;; [2]
firstpair:
   inc hl          ;; [2] 
   dec de          ;; [2]

firstbyte:
   call switch_bytes ;; [5+27]
   inc hl            ;; [2]
   inc de            ;; [2]
   call switch_bytes ;; [5+27]

   ;; C is the counter of pairs of bytes to be flipped and switched. 
   dec c           ;; [1]   C-- (one less pair of bytes to be flipped and switched)
   jr nz, nextpair ;; [2/3] If C != 0, there are still bytes to be flipped and switched, so continue

   ;; All bytes in the present row have been flipped and switched. 
   ;; Recover BC (Height/Width) form the stack, decrement height and check if there are
   ;; still more rows to be flipped
   pop  bc      ;; [3]   BC contains (B: height, C: width of the sprite)
   djnz nextrow ;; [3/4] B-- (Height--). If B!=0, there are still more rows pending, so continue

   ret          ;; [3] All sprite rows flipped. Return.

  ;; Subroutine to invert and switch a pair of bytes
  ;;
switch_bytes:
   ;; Flip byte pointed by DE
   ld a, (de)      ;; [2] A = Byte pointed by DE (2 pixels)
   ld b, a         ;; [1] B = A, Copy of A, required by reverse macro
   
   ;; Reverse (flip) both pixels contained in A (using B as temporary storage)
   cpctm_reverse_mode_0_pixels_of_A b ;; [7] 

   ;; Switch DE flipped byte and HL byte to be flipped
switch_bytes_single:
   ld b, (hl)      ;; [2] B = Byte pointed by HL (2 pixels)
   ld (hl), a      ;; [2] Store flipped byte from (DE) at (HL)

   ;; Flip byte pointed by HL and store it at (DE)
   ld a, b         ;; [1] A = B = Copy of byte pointed by HL (2 pixels)

   ;; Reverse (flip) both pixels contained in A (using B as temporary storage)
   cpctm_reverse_mode_0_pixels_of_A b ;; [7]

   ld (de), a      ;; [2] Store flipped byte from (HL) at (DE)
  
   ret             ;; [3]