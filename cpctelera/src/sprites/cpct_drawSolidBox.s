;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

.include /sprites.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawSolidBox
;;
;;    Fills up a rectangle in video memory (or screen buffer) with a given 
;; colour data byte. Could be used for drawing coloured rectangles as well as
;; erasing screen rectangles easily.
;;
;; C Definition:
;;    void *cpct_drawSolidBox* (void* *memory*, u8 *colour_pattern*, u8 *width*, u8 *height*);
;;
;; Input Parameters (5 bytes):
;;  (2B DE) memory         - Video memory pointer to the upper left box corner byte
;;  (1B _ ) colour_pattern - 1-byte colour pattern (in screen pixel format) to fill the box with
;;  (1B C ) width          - Box width *in bytes* [1-64] (Beware! *not* in pixels!)
;;  (1B B ) height         - Box height in bytes (>0)
;;
;; Parameter Restrictions:
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *colour_pattern* could be any 8-bit value, and should be in screen pixel format.
;; Functions <cpct_px2byteM0> and <cpct_px2byteM1> could be used to calculate 
;; screen pixel formatted bytes out of firmware colours for each pixel in the byte. 
;; If you wanted to know more about screen pixel formats, check <cpct_px2byteM0> or 
;; <cpct_px2byteM1>. Screen pixel format for Mode 2 is just a linear 1-pixel = 1-bit.
;;  * *width* must be the width of the box *in bytes*, and must be in the range [1-64].
;; A box *width* outside the range [1-64] will probably make the program hang or crash, 
;; due to the optimization technique used. Always remember that the *width* must be 
;; expressed in bytes and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the box in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a box in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;
;; Known limitations:
;;    * This function does not do any kind of boundary check or clipping. If you 
;; try to draw boxes on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;    * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned boxes. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;
;; Details:
;;    This function draws a solid colour-patterned box (a rectangle full
;; of a given colour pattern) anywhere at the video memory or screen buffer.
;; It does so by copying the colour pattern byte to the top-left byte 
;; of the box and then cloning that byte to the next bytes of the box.
;; As it does so using an unrolled LDIR and a dynamic JR, it is limited 
;; to 64 LDIs (64 bytes-wide at most). 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    54 bytes
;;
;; Time Measures:
;; (start code)
;; Case     |           Cycles               |      microSecs (us)
;; ----------------------------------------------------------------------------------
;; Best     |  81 + (107 + 16W)H + 40[H / 8] | 20.25 + (26.75 + 4W)H + 10*[H / 8]
;; Worst    | 121 + (107 + 16W)H + 40[H / 8] | 30.25 + (26.75 + 4W)H + 10*[H / 8]
;; ----------------------------------------------------------------------------------
;; W=2,H=16 |        2345 / 2385             |     586.25 /  596.25
;; W=4,H=32 |        5673 / 5713             |    1418.25 / 1428.25
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


_cpct_drawSolidBox::
   ;; GET Parameters from the stack
   ld  hl, #2         ;; [10] HL Points to SP+2 (first 2 bytes are return address)
   add hl, sp         ;; [11]    , to use it for getting parameters from stack
   ld   e, (hl)       ;; [ 7] DE = First Parameter (Video memory pointer)
   inc hl             ;; [ 6]
   ld   d, (hl)       ;; [ 7]
   inc hl             ;; [ 6]  / Copy first value to video memory (upper-left corner of the box)
   ldi                ;; [16] (HL)->(DE) Move second parameter (1-byte Colour Pattern) directly into first byte of the box in memory
   ld   c, (hl)       ;; [ 7] C = Third Parameter (Box Width)
   inc hl             ;; [ 6]
   ld   b, (hl)       ;; [ 7] B = Fourth Parameter (Box Height)

   ;; Prepare HL and DE pointers for copying bytes consecutively
   ld   h, d          ;; [ 4] HL = DE - 1 (HL Points to the first byte of the box, the one that contains the colour pattern)
   ld   l, e          ;; [ 4]
   dec  hl            ;; [ 6]
   push hl            ;; [11] Save HL (Pointer to the first byte of the box) for later use

   ;; Modify code using width to jump in drawSpriteWidth
   dec c                       ;; [ 4] The first line of bytes has 1 byte less to be copied (the first value we have already copied)
   ld  a, #126                 ;; [ 7] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   sub c                       ;; [ 4]    to do as much LDIs as bytes the Sprite is wide
   sub c                       ;; [ 4]
   ld (dsb_drawFirstLine+3), A ;; [13] Modify JR data to create the jump we need
   jp dsb_drawFirstLine        ;; [10] Jump to the code required for the first line (different from next lines)

   ;; Draw a sprite-line of n bytes 
dsb_drawSpriteWidth:
   push de              ;; [11] Save DE pointer, at the start of this next line to be copied
   ldi                  ;; [16] Copy the first byte, as next code is configured for width-1 bytes
dsb_drawFirstLine:
   ld c, #0xFF          ;; [ 7] C = 255 to ensure B never gets modified by LDIs (that discount 1 from BC)
   .DW #0x0018  ;; JR 0 ;; [12] Self modifying instruction: the '00' will be substituted by the required jump forward.
                        ;;      (Note: Writing JR 0 compiles but later it gives odd linking errors)
   ldi                  ;; [16] <| 63 LDIs, which are able to copy up to 63 bytes each time.
   ldi                  ;; [16]  | That means that each Sprite line should be 63 bytes width at most.
   ldi                  ;; [16]  | The JR instruction at the start makes us ignore the LDIs we don't need (jumping over them)
   ldi                  ;; [16] <| That ensures we will be doing only as much LDIs as bytes our sprite is wide
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |

   pop hl               ;; [10] HL = Pointer to the start of the last completed line (was saved previously with push de)

   dec b                ;; [ 4] Another line finished: we discount it from B (Height)
   ret z                ;; [11/5] If that was the last line, we safely return

   ;; Set up HL and DE pointers for copying next line of bytes
   ;; HL will point to the start of the last completed line and DE to the start of the next

   ;; Next pixel line is +0x800 bytes away (7 out of every 8 times)
   ld   e, l                   ;; [ 4] To make DE = HL (previous to incrementing it to the next line), we first copy E = L
   ld   a, h                   ;; [ 4] D = H + 0x08 (Add 0x800 to DE, which is equal to HL, because E = L)
   add #8                      ;; [ 7]
   ld   d, a                   ;; [ 4]
                               ;; Check if DE (next line) is first line of a screen character (8-byte aligned)...
   and #0x38                   ;; [ 7] ...by checking bits xx000xxxx. If the 3 of them are 0, it is the start of a new...
   jp  nz,dsb_drawSpriteWidth  ;; [10] ...screen character, and next pixel line is -0xFB0 (+0xC050) bytes away... 
                               ;;      ...else, DE already points to next pixel line, with previoys +0x800 bytes added.      
                               
   ;; Next pixel line is -0xFB0 (+0xC050) bytes away
dsb_next8block:
   ld   a, e              ;; [ 4] DE = DE + 0xC050 (DE Points to next pixel line)
   add #0x50              ;; [ 7]
   ld   e, a              ;; [ 4]
   ld   a, d              ;; [ 4]
   adc #0xC0              ;; [ 7]
   ld   d, a              ;; [ 4]
   jp dsb_drawSpriteWidth ;; [10] Draw next pixel line