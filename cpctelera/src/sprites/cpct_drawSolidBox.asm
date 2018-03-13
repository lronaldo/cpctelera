;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawSolidBox
;;
;;    Fills up a rectangle in video memory (or screen buffer) with a given 
;; colour data byte. Could be used for drawing coloured rectangles as well as
;; erasing screen rectangles easily.
;;
;; C Definition:
;;    void <cpct_drawSolidBox> (void* *memory*, <u8> *colour_pattern*, <u8> *width*, <u8> *height*);
;;
;; Input Parameters (5 bytes):
;;  (2B DE) memory         - Video memory pointer to the upper left box corner byte
;;  (1B A ) colour_pattern - 1-byte colour pattern (in screen pixel format) to fill the box with
;;  (1B C ) width          - Box width *in bytes* [1-64] (Beware! *not* in pixels!)
;;  (1B B ) height         - Box height in bytes (>0)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSolidBox_asm
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
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
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
;;    C-bindings - 184 bytes
;;  ASM-bindings - 172 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |      microSecs (us)      |      CPU Cycles
;; ---------------------------------------------------------------------
;;  Best      |  35 + (29 + 5W)H  + 11HH | 140 + (116 + 20W)H  + 44HH
;;  Worst     |        Best + 11         |      Best + 44
;; ---------------------------------------------------------------------
;;  W=2,H=16  |         670 /  681       |     2680 / 2724
;;  W=4,H=32  |        1636 / 1647       |     6544 / 6588
;; ---------------------------------------------------------------------
;; Asm saving |         -23              |        -92
;; ---------------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   push  hl         ;; [4] Save HL (Pointer to the first byte of the box) for later use

   ;; Modify code using width to jump in drawSpriteWidth
   dec   c          ;; [1] The first line of bytes has 1 byte less to be copied (the first value we have already copied)
   ld    a, #126    ;; [2] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   sub   c          ;; [1]    to do as much LDIs as bytes the Sprite is wide
   sub   c          ;; [1]
   ld (dsb_drawFirstLine+3), a ;; [5] Modify JR data to create the jump we need
   jr dsb_drawFirstLine        ;; [3] Jump to the code required for the first line (different from next lines)

   ;; Draw a sprite-line of n bytes 
dsb_drawSpriteWidth:
   push de          ;; [4] Save DE pointer, at the start of this next line to be copied
   ldi              ;; [5] Copy the first byte, as next code is configured for width-1 bytes
dsb_drawFirstLine:
   ld c, #0xFF      ;; [2] C = 255 to ensure B never gets modified by LDIs (that discount 1 from BC)
   jr__0            ;; [3] Self modifying instruction: the '00' will be substituted by the required jump forward.
                    ;;      (Note: Writing JR 0 compiles but later it gives odd linking errors)
   ldi              ;; [5] <| 63 LDIs, which are able to copy up to 63 bytes each time.
   ldi              ;; [5]  | That means that each Sprite line should be 63 bytes width at most.
   ldi              ;; [5]  | The JR instruction at the start makes us ignore the LDIs we don't need (jumping over them)
   ldi              ;; [5] <| That ensures we will be doing only as much LDIs as bytes our sprite is wide
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

   pop hl           ;; [3] HL = Pointer to the start of the last completed line (was saved previously with push de)

   dec b            ;; [1] Another line finished: we discount it from B (Height)
   ret z            ;; [2/4] If that was the last line, we safely return

   ;; Set up HL and DE pointers for copying next line of bytes
   ;; HL will point to the start of the last completed line and DE to the start of the next

   ;; Next pixel line is +0x800 bytes away (7 out of every 8 times)
   ld   e, l                   ;; [1] To make DE = HL (previous to incrementing it to the next line), we first copy E = L
   ld   a, h                   ;; [1] D = H + 0x08 (Add 0x800 to DE, which is equal to HL, because E = L)
   add #8                      ;; [2]
   ld   d, a                   ;; [1]
                               ;; Check if DE (next line) is first line of a screen character (8-byte aligned)...
   and #0x38                   ;; [1] ...by checking bits xx000xxxx. If the 3 of them are 0, it is the start of a new...
   jp  nz,dsb_drawSpriteWidth  ;; [3] ...screen character, and next pixel line is -0xFB0 (+0xC050) bytes away... 
                               ;;     ...else, DE already points to next pixel line, with previoys +0x800 bytes added.      
                               
   ;; Next pixel line is -0xFB0 (+0xC050) bytes away
dsb_next8block:
   ld   a, e              ;; [1] DE = DE + 0xC050 (DE Points to next pixel line)
   add #0x50              ;; [2]
   ld   e, a              ;; [1]
   ld   a, d              ;; [1]
   adc #0xC0              ;; [2]
   ld   d, a              ;; [1]
   jp dsb_drawSpriteWidth ;; [3] Draw next pixel line