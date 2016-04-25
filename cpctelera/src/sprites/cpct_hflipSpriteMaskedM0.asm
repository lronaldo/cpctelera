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
;; Function: cpct_hflipSpriteMaskedM0
;;
;;   Horizontally flips a sprite, encoded in screen pixel format, *mode 0*,  with 
;; interlaced mask.
;;
;; C definition:
;;   void <cpct_hflipSpriteMaskedM0> (<u8> width, <u8> height, void* sprite) __z88dk_callee;
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
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; with interlaced mask data (Pairs of bytes, 1st: mask data for next byte, 2nd:
;; mode 0 pixel data byte). You may check screen pixel format for mode 0 (<cpct_px2byteM0>).
;; Each mask byte will contain enabled bits as those that should be picked from the background
;;  (transparent) and disabled bits for those that will be printed from sprite colour data. 
;; Each mask data byte must precede its associated colour data byte. Sprite must be rectangular 
;; and all bytes in the array must be consecutive pixels, starting from top-left corner 
;; and going left-to-right, top-to-bottom down to the bottom-right corner. Total amount 
;; of bytes in pixel array should be 2 x *width* x *height*. 
;;  * *width* must be the width of the sprite *in bytes*, without accounting for mask bytes, 
;;  and *must be 1 or more*. Using 0 as *width* parameter for this function could potentially 
;; make the program hang or crash. Always remember that the *width* must be expressed in bytes
;; and *NOT* in pixels. The correspondence is 1 byte = 2 pixels (mode 0)
;;  * *height* must be the height of the sprite in pixels / bytes (both should be the
;; same amount), and must be greater than 0. There is no practical upper limit to this value,
;; but giving a height greater than the height of the sprite will yield undefined behaviour,
;; as bytes after the sprite array might result modified. 
;;
;; Known limitations:
;;  * This function will admit any 16-bits value as *sprite* pointer and will *modify* 
;; bytes at the given address. On giving an incorrect address this function will yield 
;; undefined behaviour, probably making your program crash or doing odd things due to 
;; part of the memory being altered by this function.
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
;;    This function performs an horizontal flipping of the given *sprite* along with its
;; interlaced mask. To do so, the function inverts the order of the pairs of bytes formed
;; by each mask byte and its associated pixel-definition byte, for all sprite rows. It also 
;; inverts pixel order inside each byte of the *sprite* (be it pixel byte or mask byte). 
;; The function expects to receive an *sprite* in screen pixel format (mode 0), along with
;; its interlaced mask like in this example:
;;
;; (start code)
;;    // Example call. Sprite has 8x4 pixels (4x4 bytes)
;;    cpct_hflipSpriteMaskedM0(4, 4, sprite);
;;
;;    // Operation performed by the call and results (mM, nN = mask data bytes)
;;    //
;;    // ---------------------------------------------------------------------------------------------
;;    //  |          Received as parameter             |          Result after flipping              |
;;    // ---------------------------------------------------------------------------------------------
;;    //  | sprite => [mM][05][nN][21][mM][73][nN][40] |  sprite => [Nn][04][Mm][37][Nn][12][Mm][05] |
;;    //  |           [mM][52][nN][23][mM][37][nN][74] |            [Nn][47][Mm][73][Nn][32][Mm][25] |
;;    //  |           [mM][05][nN][11][mM][31][nN][04] |            [Nn][04][Mm][13][Nn][11][Mm][50] |
;;    //  |           [mM][00][nN][55][mM][44][nN][00] |            [Nn][00][Mm][44][Nn][55][Mm][00] |
;;    // ---------------------------------------------------------------------------------------------
;;    //  Sprite takes 32 consecutive bytes in memory: 4 rows with 8 bytes (4 bytes pixel data, 
;;    //  and 4 bytes mask data)
;;    //
;; (end code)
;;
;;    As can be seen on the example, the function modifies the bytes of the *sprite* in-place.
;; Therefore, the *sprite* becomes horizontally flipped after the call and will not return to 
;; normal status unless another horizontally flipping operation is performed.
;;
;;    This function performs reasonably well compared to <cpct_hflipSpriteM0>, as time required
;; for doing the flip is a little bit less than doubled, for double amount of bytes. However, 
;; for maximum performance, functions making use of memory-aligned conversion tables are advised.
;; Also, having memory-aligned sprites will permit developing even faster versions. In any case,
;; The most important advantage of this function is its reduced size, compared to requiring a
;; function and a 256-bytes aligned table together.
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
;;          cpct_hflipSpriteMaskedM0(4, 8, characterSprite);
;;          wasLookingAt = lookingAt;
;;       }
;;
;;       // Draw main character's sprite
;;       pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
;;       cpct_drawSpriteMasked(characterSprite, pvmem, 4, 8);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 62 bytes
;;  ASM-bindings - 59 bytes
;;
;; Time Measures:
;; (start code)
;;  Case       |      microSecs (us)       |         CPU Cycles          |
;; -----------------------------------------------------------------------
;;  Even-width |     (80WW + 18)H + 16     |     (320WW +  72)H + 64     |
;;   Odd-width |     (80WW + 69)H + 16     |     (320WW + 244)H + 64     |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |          1584             |           6336              |
;;  W=5,H=32   |          7344             |          29376              |
;; -----------------------------------------------------------------------
;;  Asm saving |          -12              |            -48              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *WW* = (int)(*width*/2), *H* = *height*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  
   ;; We need HL to point to the byte previous to the start of the sprite array.
   ;; This is because HL will point to the previous byte of the next row in the 
   ;; array at the end of every loop, and hence we can add always the same quantity
   ;; (sprite width) to point to the centre of next row.
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
;; ------------------------------------------------------------------------------------
;; byte order    |   A   B   C   D       C   D   A   B
;; pixel values  | [mM][12][nN][34]--> [Nn][43][Mm][21]  :: mM & nN are mask nibbles
;; ------------------------------------------------------------------------------------
;; byte layout   | [ 0123 4567 ]     [ 1032 5476 ] << Bit order (reversed by pairs)
;; (Mode 0,2 px) | [ 0101 0101 ] --> [ 1010 1010 ] << Pixel bits (pixel 1, pixel 0)
;; ------------------------------------------------------------------------------------
;;
nextrow:
   push bc      ;; [4] Save Height/Width into the stack for later recovery
   ld   b, #0   ;; [2] BC = C = Width (In order to easily add it to HL)
   add  hl, bc  ;; [3] HL += BC (Add Width to start-of-row pointer, HL, to make it point to the central byte)
                ;; ... Remember that sprite Width is half the width of the array, as it does not account for masks
   ld   d, h    ;; [1] | DE = HL, so that DE also points to the central byte of the row
   ld   e, l    ;; [1] | 
   srl  c       ;; [2] C = C/2 (Sprite width/2, to act as counter of loops, each loop switching 2 bytes 
                ;; ... and their respective masks, for a total of 4 bytes reversed and switched per loop)

   ;; Even sprites jump from here directly to firstpair, where HL and DE are moved to point to 
   ;; both central bytes, like in this diagram: (P: pixel byte, M: mask byte)
   ;; --------------------------------------------------------------------------------------------------
   ;;   row bytes->> |[M][P][M][P][M][P][M][P]|[M][P][M][P][M][P][M][P]| 8 pixels per row
   ;;   Width: 4     |          |             |          |  |          | 4 pixel bytes (2 pixels/byte)
   ;;                |        HL&DE      -->> | -->>     DE HL         | 4 mask  bytes
   ;; --------------------------------------------------------------------------------------------------
   jr  nc, firstpair        ;; [2/3] If Carry=0, last bit of C was 0, so Width is Even, then jump

   ;; Odd sprites first switch the central byte, then start with the rest, 
   ;; according to this diagram: (P: pixel byte, M: mask byte, p/m: pixel/mask reversed byte)
   ;; --------------------------------------------------------------------------------------------------
   ;;   row bytes->> |[M][P][M][P][M][P]|[M][P][m][p][M][P]| 6 pixels per row
   ;;   Width: 3     |       |          |          |       | 3 pixel bytes (2 pixels per byte)
   ;;                |      HL&DE       |        HL&DE     | 3 mask  bytes
   ;; --------------------------------------------------------------------------------------------------
   call switch_bytes_single ;; [5+17] Reverse central pair, mask definition byte (first byte)
   inc  hl                  ;; [2]    HL++ (HL Points to second central byte, pixel definition)
   inc  de                  ;; [2]    DE++ (DE also points to second central byte, pixel definition)
   call switch_bytes_single ;; [5+17] Reverse central pair, pixel definition byte (second byte)

   ;;
   ;; INTERNAL LOOP THAT FLIPS THE SPRITE
   ;;    Iterates throw all the bytes inside each row
   ;;
   ;;    It takes pairs of bytes 2-by-2, one pointed by DE and the other pointed by HL. 
   ;; It flips pixels inside each byte and its subsequent mask (the next byte in the pair) 
   ;; and then switches both pairs of bytes pointed by DE and HL. 
   ;;    After switching each pair of bytes, HL is increased to point to the next pair 
   ;; of bytes (first byte of the pair) and DE is decremented by 3, to point to the 
   ;; first byte of the pair that needs to be switched with that pointer by HL.
   ;;
nextpair:
   dec de            ;; [2] | DE -= 2 (Make DE go backwards 1 pair of bytes, but still
   dec de            ;; [2] |  point to the second byte of the pair)
firstpair:
   dec de            ;; [2] DE-- (Make DE point to the first byte of its pair of bytes)
   inc hl            ;; [2] HL++ (Point to the next pair of bytes)

firstbyte:
   call switch_bytes ;; [5+27] Reverse and Switch first byte of both pairs of bytes (pixel values)
   inc hl            ;; [2]    HL++ (HL points to the second byte of the pair, its mask)
   inc de            ;; [2]    DE++ (DE points to the second byte of the pair, its mask)
   call switch_bytes ;; [5+27] Reverse and Switch second byte of both pairs of bytes (mask values)

   ;; C is the counter of pairs of bytes to be flipped and switched. 
   dec c             ;; [1]   C-- (one less pair of bytes to be flipped and switched)
   jr nz, nextpair   ;; [2/3] If C!=0, there are still pairs of bytes to be flipped and switched, so continue

   ;; All pairs of bytes in the present row have been flipped and switched. 
   ;; Recover BC (Height/Width) form the stack, decrement height and check if there are
   ;; still more rows to be flipped
   pop  bc           ;; [3]   BC contains (B: height, C: width of the sprite)
   djnz nextrow      ;; [3/4] B-- (Height--). If B!=0, there are still more rows pending, so continue

   ret               ;; [3] All sprite rows flipped. Return.

  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; switch_bytes
  ;;     Subroutine to invert and switch a pair of bytes in mode 0
  ;;
  ;; INPUTS:
  ;;   HL: Pointer to Byte 1 to be inverted and switched with DE
  ;;   DE: Pointer to Byte 2 to be inverted and switched with HL
  ;;
  ;; Modifies:
  ;;   AF, B
  ;;
  ;; In-between entry-point:
  ;;   switch_bytes_single - Gets byte pointed by HL, reverses it and moves it to DE
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
  
   ret             ;; [3] Bytes switched and reversed, return