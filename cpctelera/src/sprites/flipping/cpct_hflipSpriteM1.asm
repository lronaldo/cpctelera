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
;; Function: cpct_hflipSpriteM1
;;
;;   Horizontally flips a sprite, encoded in screen pixel format, *mode 1*.
;;
;; C definition:
;;   void <cpct_hflipSpriteM1> (<u8> width, <u8> height, void* sprite) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (1B  C) width  - Width of the sprite in *bytes* (*NOT* in pixels!). Must be >= 1.
;;  (1B  B) height - Height of the sprite in pixels / bytes (both are the same). Must be >= 1.
;;  (2B HL) sprite - Pointer to the sprite array (first byte of consecutive sprite data)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_hflipSpriteM1_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; (Mode 1 data, 4 pixels per byte). Sprite must be rectangular and all bytes in the array 
;; must be consecutive pixels, starting from top-left corner and going left-to-right, top-to-bottom 
;; down to the bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*
;; You may check screen pixel format for mode 1 (<cpct_px2byteM1>). *Warning!* This function
;; will admit any 16-bits value as sprite pointer and will *modify* bytes at the given
;; address. On giving an incorrect address this function will yield undefined behaviour,
;; probably making your program crash or doing odd things due to part of the memory being
;; altered by this function.
;;  * *width* must be the width of the sprite *in bytes* and must be 1 or more. 
;; Using 0 as *width* parameter for this function could potentially make the program 
;; hang or crash. Always remember that the *width* must be expressed in bytes 
;; and *not* in pixels. The correspondence is 1 byte = 4 pixels (mode 1)
;;  * *height* must be the height of the sprite in pixels / bytes (both should be the
;; same amount), and must be greater than 0. There is no practical upper limit to this value,
;; but giving a height greater than the height of the sprite will yield undefined behaviour,
;; as bytes after the sprite array might result modified. 
;;
;; Known limitations:
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
;; an sprite in screen pixel format (mode 1), like in this example:
;;
;; (start code)
;;    // Example call. Sprite has 16x4 pixels (4x4 bytes)
;;    cpct_hflipSpriteM1(4, 4, sprite);
;;
;;    // Operation performed by the call and results
;;    //
;;    // -----------------------------------------------------------------------------
;;    //  |  Received as parameter             | Result after flipping               |
;;    // -----------------------------------------------------------------------------
;;    //  | sprite => [0123][5678][HGFE][DCBA] |  sprite => [ABCD][EFGH][8765][3210] |
;;    //  |           [9876][5432][abcd][efgh] |            [hgfe][dcba][2345][6789] |
;;    //  |           [0123][5678][HGFE][DCBA] |            [ABCD][EFGH][8765][3210] |
;;    //  |           [9876][5432][abcd][efgh] |            [hgfe][dcba][2345][6789] |
;;    // -----------------------------------------------------------------------------
;;    //  Sprite takes 16 consecutive bytes in memory (4 rows with 4 bytes, 
;;    //  and 4 pixels each byte, for a total of 16x4 pixels, 64 pixels)
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
;;    Next example shows a function designed to draw an animated flag
;; that flips right-to-left as if it was being agitated. To do so, every
;; 30 times it is redrawn, it is flipped horizontally to simulate that 
;; right-to-left animated movement: 
;; (start code)
;;    // Draws an animated flag that changes from left-to-right
;;    // periodically (every 30 times it is redrawn). Flag is
;;    // 24x16 mode 1 pixels (6x16 bytes)
;;    void drawFlag(u8 x, u8 y) {
;;       static u8 timesdrawn;  // Statically count the number of times the flag has been drawn
;;       u8* pvmem;            // Pointer to video memory to draw the sprite
;;
;;       // Count for a new redraw of the flag and check if it has been
;;       // drawn 30 or more times in order to flip it
;;       if(++timesdrawn > 30) {
;;          // Horizontally flip the flag to animate it
;;          cpct_hflipSpriteM1(6, 16, flagSprite);
;;          timesdrawn = 0;
;;       }
;;
;;       // Draw the flag
;;       pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
;;       cpct_drawSprite(flagSprite, pvmem, 6, 16);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 86 bytes
;;  ASM-bindings - 83 bytes
;;
;; Time Measures:
;; (start code)
;;  Case       |      microSecs (us)       |         CPU Cycles          |
;; -----------------------------------------------------------------------
;;  Even-width |     (48WW + 16)H + 32     |     (192WW +  64)H + 128    |
;;  Oven-width |     (48WW + 36)H + 37     |     (192WW + 144)H + 148    |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |          1056             |           4224              |
;;  W=5,H=32   |          4261             |          17044              |
;; -----------------------------------------------------------------------
;;  Asm saving |          -12              |            -48              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *WW* = *width*/2, *H* = *height*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;
;; USEFUL CONSTANTS TO CLARIFY CODE
;;    ld_b_hl - Opcode for Z80 operation "LD B, (HL)"
;;    jr_one  - Opcode for Z80 operation "JR one". It must be 0x??18, as 0x18 is 
;;              the opcode for JR and ?? is the offset from (varjump+3) to (one)
;;              (NOTE: (varjump+3) refers to the byte right after this "JR one" operation, 
;;               taking into account that ld_b_hl goes before "JR one" and takes up 1 byte)
;;    jr_byteloop - Opcode for Z80 operation "JR nextbyte+1". It is generated same as
;;                  jr_one, but to jump from (varjump+2) to (nextbyte+1), as there is no ld_b_hl
;;                  operation before jr_byteloop.
;;
ld_b_hl     = 0x46
jr_one      = 0x18 + 0x100 * (          one  - (varjump + 3))
jr_byteloop = 0x18 + 0x100 * ((nextbyte + 1) - (varjump + 2))
   
;;
;; SET UP CODE: Set up loop internal "variable jump" {varjump} code so that it takes
;;    into account the differences between odd-wide and even-wide sprites. 
;;    - Even-width sprites flip 2 bytes per each loop iteration (and switches them)
;;    - Odd-width sprites have a first iteration of the loop in which only the 
;;      central byte is flipped (and then leaved in its same place)
;;

   ;; We need HL to point to the byte previous to the start of the sprite array.
   ;; This is because HL will point to the previous byte of the next row in the 
   ;; array at the end of every loop, and hence we can add always the same quantity
   ;; (half sprite width, with +1 for odd-width sprites) to point to the centre of next row.
   dec hl             ;; [2] HL points to the byte previous to the start of the sprite array

   ;; Divide sprite width by 2 and check if there is remainder to know if the 
   ;; sprite is odd-wide (if remainder, then Carry = 1)
   srl c              ;; [2]   C = C / 2 (Sprite width / 2)
   jr  c, odd_width   ;; [2/3] Carry = 1, means Odd sprite width

   ;; For even sprite widths, the only thing we will need to do will be to increase
   ;; HL pointer 1 each row. This is because both HL and DE will be in this situation:
   ;;---------------------------------------------------------------------------------
   ;;       Sprite Row => [B1][B2][B3][B4] == Increase HL ==> [B1][B2][B3][B4]
   ;;                          |                                   |   |
   ;;                        HL&DE                                 DE  HL
   ;;---------------------------------------------------------------------------------
   ;; Then Leaving DE and HL pointing to both central bytes of the sprite array will 
   ;; let the loop proceed reversing and switching them. To to this, we put a JR #first-1
   ;; operation in the varjump zone, that will jump to next INC HL operation.
   ;;
even_width:
   ld  de, #jr_byteloop ;; [3] DE = Opcode for "JR nextbyte+1" operation, to jump to the byte loop
   ld  (varjump), de    ;; [6] Insert "JR nextbyte+1" at varjump location
   jr nextrow           ;; [3] Start processing the loop for the first 2 bytes
   
   ;; For odd sprite widths, we need to set up a first loop iteration that only takes
   ;; the central byte of the sprite row and reverses it. So, as both HL & DE will be
   ;; pointing to the central byte, we add an operation to read the byte in B (LD B, (HL))
   ;; and jump to the middle of the loop (label "one"), just to perform the reversing operation
odd_width: 
   ld   a, #ld_b_hl   ;; [2] A = Opcode for "LD B, (HL)" operation
   ld (varjump), a    ;; [4] Insert "LD B, (HL)" at varjump location
   ld  de, #jr_one    ;; [3] DE = Opcode for "JR one" operation
   ld (varjump+1), de ;; [6] Insert "JR one" at varjump+1 location (right after LD B, (HL))
   inc c              ;; [1] C++ (C = Width + 1, as reversing the central byte will require 1 more looping...
                      ;;     ... and also the central byte will be at [HL + Width/2 + 1] )
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
;; byte order    |   A     B     C          C     B     A
;; pixel values  | [1234][5678][90AB] --> [BA09][8765][4321]
;; --------------------------------------------------------------------------
;; byte layout   | [ 0123 4567 ]     [ 3210 7654 ] << Bit order (reversed by pairs)
;; (Mode 1, 4 px)| [ abcd abcd ] --> [ dcba dcba ] << Pixel bits (4 pixels, a-d)
;;
nextrow:
   push bc      ;; [4] Save Height/Width into the stack for later recovery
   ld   b, #0   ;; [2] BC  = C = Width/2 (In order to easily add the width to HL)
   add  hl, bc  ;; [3] HL += BC (Add Width/2 to start-of-row pointer, HL, to make it point to the central byte)
   ld   d, h    ;; [1] | DE = HL, so that DE also points to the central byte of the row
   ld   e, l    ;; [1] | 

   ;;
   ;; VARIABLE JUMP
   ;;    This jump is a piece of self-modifying code that changes depending on the
   ;; width of the sprite. For odd-width sprites this code will load the central byte into
   ;; B and jump to label one { LD B, (HL) : JR one } (3 bytes). For even-width sprites
   ;; it will just jump to label nextbyte plus 1 { JR nextbyte+1 } which will do a INC HL operation
   ;; and then start performing normal reverse and switch operations to flip the sprite.
   ;;
varjump:
   ;; Odd-width varjump code (3-bytes, longer one)
   ld   b, (hl) ;; [2] B = central byte of the row, ready to be reversed
   jr   one     ;; [3] Jump to the second part of the internal loop, to reverse only the central byte

   ;; Even-width varjump code (2-bytes)
;; jr  nextbyte+1  ;; [3] Jump to the instruction previous to the inner loop (INC HL)

   ;;
   ;; INTERNAL LOOP THAT FLIPS THE SPRITE
   ;;    Iterates throw all the bytes inside each row
   ;;
   ;;    If takes byte 2-by-2, one pointed by DE and the other pointed by HL. It flips
   ;; pixels inside each byte and then switches both bytes pointed by DE and HL. There is
   ;; a special case for the central byte, at the label "one", which only flips pixels
   ;; in that byte and leaves it at the same place.
   ;;
nextbyte:
   dec de          ;; [2] DE-- (Move DE from the central byte to the first of the row)
   inc hl          ;; [2] HL++ (Move HL from the central byte to the last of the row)

   ;; Flip byte pointed by DE
   ld a, (de)      ;; [2] A = Byte pointed by DE (4 pixels)
   ld b, a         ;; [1] B = A, Copy of A, required by reverse macro
   
   ;; Reverse (flip) the 4 pixels contained in A (Using B as temporary storage)
   cpctm_reverse_mode_1_pixels_of_A b ;; [16] 

   ;; Switch DE flipped byte and HL byte to be flipped
   ld b, (hl)      ;; [2] B = Byte pointed by HL (4 pixels)
   ld (hl), a      ;; [2] Store flipped byte from (DE) at (HL)

   ;; Flip byte pointed by HL and store it at (DE)
one:
   ld a, b         ;; [1] A = B = Copy of byte pointed by HL (4 pixels)

   ;; Reverse (flip) the 4 pixels contained in A (using B as temporary storage)
   cpctm_reverse_mode_1_pixels_of_A b ;; [16]

   ld (de), a      ;; [2] Store flipped byte from (HL) at (DE)

   ;; C is the counter of pairs of bytes to be flipped and switched. 
   dec c           ;; [1]   C-- (one less pair of bytes to be flipped and switched)
   jr nz, nextbyte ;; [2/3] If C != 0, there are still bytes to be flipped and switched, so continue


   ;; All bytes in the present row have been flipped and switched. 
   ;; Recover BC (Height/Width) form the stack, decrement height and check if there are
   ;; still more rows to be flipped
   pop  bc      ;; [3]   BC contains (H: height, L: width of the sprite)
   djnz nextrow ;; [3/4] B-- (Height--). If B!=0, there are still more rows pending, so continue


   ret          ;; [3] All sprite rows flipped. Return.
