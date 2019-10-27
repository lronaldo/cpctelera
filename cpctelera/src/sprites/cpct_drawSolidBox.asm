;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2019 Arnaud bouche (@Arnaud6128)
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
;;  (2B HL) memory         - Video memory pointer to the upper left box corner byte
;;  (2B E ) colour_pattern - 1-byte colour pattern (in screen pixel format) to fill the box with
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
;; As it does so using a dynamic JR, the Rectangle Width is limited 
;; to 64 Bytes (64 bytes-wide at most). 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 168 bytes
;;  ASM-bindings - 161 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |      microSecs (us)      |      CPU Cycles
;; ---------------------------------------------------------------------
;;  Best      |  13 + (18 + 4W)H + 9HH   | 60 + (72 + 16W)H  + 36HH
;;  Worst     |        Best + 11         |      Best + 44
;; ---------------------------------------------------------------------
;;  W=2,H=16  |         443 / 451        |     1772 / 1804
;;  W=4,H=32  |        1133 / 1141       |     4532 / 4564
;; ---------------------------------------------------------------------
;; Asm saving |         -19              |        -76
;; ---------------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.macro COPY_E_TO_HL
   ld (hl), e             ;; [2] (HL) = E
   inc hl                 ;; [2] HL++  
.endm
 
   ld   d, b              ;; [1] D = B Height (used as counter for the number of lines we have to fill)
   ld   a, c              ;; [1] A = C (Width)
   neg                    ;; [1] A = -A : Compute LSB of 0x0800 minus the Width of the sprite B to destination pointer 
   ld (dsb_drawSolidBox_width + #1), a ;; [4] Modify placeholder for ld bc, #0x07XX

   ;; Modify code using width to jump in drawSpriteWidth
   ld    a, #126           ;; [2] We need to jump 126 bytes (63 COPY_E_TO_HL*2 bytes) minus the width of the sprite * 2 (2B)
   sub   c                 ;; [1]  to do as much COPY_E_TO_HL as bytes the Sprite is wide
   sub   c                 ;; [1]
   ld (dsb_drawSolidBox_width + #4), a ;; [4] Modify JR data to create the jump we need
  						  
dsb_drawSolidBox_width:
   ;; Fill a sprite-line of n bytes
   ld   bc, #0x0700        ;; [3] Self modifying 0x07XX : compute 0x0800 minus the Width of the sprite is the distance in memory from one pixel line 
                           ;; ... to the next within every 8 pixel lines
   jr__0                   ;; [3] Self modifying instruction: the '00' will be substituted by the required jump forward. 
                           ;; ... (Note: Writting JR 0 compiles but later it gives odd linking errors)
		
   COPY_E_TO_HL            ;; [4] <| 63 COPY_E_TO_HL, which are able to copy up to 63 bytes each time.
   COPY_E_TO_HL            ;; [4]  | That means that each Sprite line should be 63 bytes width at most.
   COPY_E_TO_HL            ;; [4]  | The JR instruction at the start makes us ignore the COPY_E_TO_HL we don't need 
   COPY_E_TO_HL            ;; [4] <| (jumping over them) That ensures we will be doing only as much COPY_E_TO_HL 
   COPY_E_TO_HL            ;; [4] <| as bytes our sprite is wide.
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4] <|
   COPY_E_TO_HL            ;; [4]  |
   COPY_E_TO_HL            ;; [4]  |
 
   dec   d                 ;; [1] Another line finished: we discount it from D
   ret   z                 ;; [2/4] If that was the last line, we safely return

   ;; Jump destination pointer to the start of the next line in video memory
   add  hl, bc             ;; [3] We add 0x800 minus the width of the sprite (BC) to destination pointer 
   ld    a, h              ;; [1] We check if we have crossed video memory boundaries (which will happen every 8 lines). 
                           ;; ... If that happens, bits 13,12 and 11 of destination pointer will be 0
   and   #0x38             ;; [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   jp   nz, dsb_drawSolidBox_width ;; [3/4] If any bit from {13,12,11} is not 0, we are still inside 
                                   ;; ... video memory boundaries, so proceed with next line

   ;; Every 8 lines, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means our next line is 16K-0x50 bytes back
   ;; which is the same as advancing 48K+0x50 = 0xC050 bytes, as memory is 64K 
   ;; and our 16bit pointers cycle over it
   ld   bc, #0xC050            ;; [3] We advance destination pointer to next line
   add  hl, bc                 ;; [3]  HL += 0xC050
   jp   dsb_drawSolidBox_width ;; [3] Continue copying