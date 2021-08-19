;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2021 Joaquín Ferrero (https://github.com/joaquinferrero)
;;  Copyright (C) 2021 Nestornillo (https://github.com/nestornillo)
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_video
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_getScreenPtr
;;
;;    Returns a byte-pointer to a screen memory location, given its X, Y coordinates.
;; (in *bytes*, NOT in pixels!)
;;
;; C Definition:
;;    <u8>* <cpct_getScreenPtr> (void* *screen_start*, <u8> *x*, <u8> *y*)
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) screen_start - Pointer to the start of the screen (or a backbuffer)
;;    (1B C ) x            - [0-79]  Byte-aligned column starting from 0 (x coordinate, 
;;                           *in bytes*)
;;    (1B B ) y            - [0-199] row starting from 0 (y coordinate)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_getScreenPtr_asm
;;
;; Parameter Restrictions:
;;    * *screen_start* will normally be the start of the video memory where you want to 
;; draw something. However, you may give a pointer to the start of a screen backbuffer
;; located everywhere in memory.
;;    * *x* must be in the range [0-79]. This represents the x-coordinate (horizontal) in 
;; bytes (*NOT in pixels*) of the desired location inside the video buffer. This function 
;; assumes an standard 80-bytes-wide screen.
;;    * *y* must be in the range [0-199]. This represents the y-coordinate (vertical) in 
;; either bytes or pixels. Pixels and bytes coincide in vertical resolution. This function
;; assumes an standard 200-pixels-high screen. 
;;    * If you give incorrect values to this function, the returned pointer could
;; point anywhere in memory. This function will not cause any damage by itself, 
;; but you may destroy important parts of your memory if you use its result to 
;; write to memory, and you gave incorrect parameters by mistake. Take always
;; care.
;;
;; Return Value:
;;    (HL) u8* - Pointer to the (*x*,*y*) byte in the screen memory that starts 
;; at *screen_start*.
;;
;; Known limitations:
;;    * This function only works for default screens (80 bytes wide, 200 pixel
;; lines, 25 characters of 8 pixel lines). If you change screen configuration
;; by modifying CRTC registers, this function may not work for you.
;;
;; Details:
;;    This function returns a pointer to the byte whose screen coordinates
;; are (*x*, *y*) in *bytes*, being *screen_start* the screen origin (0, 0) or,
;; what is the same, a pointer to the first byte of the screen. The "screen"
;; may be the actual screen or a backbuffer with same layout.
;;
;;    What this function exactly does is this calculation (C code),
;;    > return screen_start + 80 * ((int)(y / 8)) + 2048 * (y % 8) + x;
;;
;;    This calculation is based on the default layout of the screen, which is
;; 80 bytes wide (80 bytes per pixel line) and 200 pixel lines high. Pixel
;; lines are grouped into 25 groups of 8 pixel lines, as it is explained in
;; <cpct_drawSprite>. Each group of 8 pixel lines starts 0x50 bytes away 
;; from the previous one (0x50 = 80, hence the term [80 * ((int)(y / 8))]) and
;; the each lines inside a group is separated by 0x800 bytes from its
;; predecessor (0x800 = 2048, and we have the other term [2048 * (y % 8)]). 
;;
;;    All the previous paragraph explains how much we have to offset from
;; *screen_start* to arrive to the start of the *y* pixel line. Once we are
;; there, the byte *x* is just *x* bytes away, so we only have to add *x*
;; to the formula and we are done.
;;
;;    With this information in mind, we can easily rename terms as follows,
;;    > Screen Character Row:             R = (int)(y / 8) 
;;    > Pixel Line inside Character Row:  L = y % 8
;;
;;    Which leave us with this more descriptive equation,
;;    > return screen_start + 80*R + 2048*L + x;
;;
;; Destroyed Register values:
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 24 bytes
;;  ASM-bindings - 20 bytes
;;
;; Time Measures: 
;; (start code)
;;     Case   | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any    |      45        |     159
;; -----------------------------------------
;; Asm saving |     -13        |     -41
;; -----------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Before starting, some terminology to understand comments,
   ;;    Screen is composed of 25 Character Rows (R: Row)
   ;;    Each Character Row is composed of 8 pixel lines (L: Line)
   ;;    CPU Registers are named with a small r in-front (rA, rB, rH, rL, rHL...)
   ;;
   ;; 1st of all, split Y-coordinate (rB) into:
   ;;    rH = B % 8         ;; <<- Line [0-7] inside the character Row
   ;;    rL = int(B / 8)    ;; <<- Screen character Row [0-24]
   ;;
   ;; We can see Y-coordinate as including this two numbers (R: Row, L: Line) 
   ;; split into its first 5-bits and its last 3-bits:
   ;;    Y-coordinate == [R|R|R|R|R|L|L|L]
   ;;                bit7-/             \-bit0
   ;;

   ;; Extract line number (L) from Y-coordinate
   ;; And multiply it by 256. rHL = 256 * L
   ld    a, b     ;; [1] rA = Y-Coordinate
   and   #0x07    ;; [2] /
   ld    h, a     ;; [1] \ rH = Y % 8      ;; << rH contains Line number [0-7] inside character Row
                  ;;     Putting Line Number into rH is similar to putting it 
                  ;;     into rHL and then shifting it 8-bits left. Same as 256*rHL.
                  ;;     Therefore, this is like doing HL += 256*L

   ;; Now extract Screen Character Row (R) from Y-Coordinate
   ld    a, b     ;; [1] rA = Y-Coordinate
   and   #0xF8    ;; [2] /
   ld    l, a     ;; [1] \ rL = 8*int(Y/8) ;; << L contains Screen Character Row multiplied by 8
                                           ;;    as bits are shifted 3-bits to the left because
                                           ;;    the 3-least-significant-bits had the line number (L)

   ;; Now rHL = 256*L + 8*R  { rH = Y%8 = L ||| rL = 8*int(Y/8) = 8*R }
   ;; We need to calculate 2048*L + 80*R
   ;; This can be factored into 8*(256*L + 10*R)
   ;; So first, make rL contain 10*R and then we will have rHL = 256*L + 10*R
   ;; And then we will only need to multiply rHL by 8

   ;; rA' = rA / 4 = 2 * int(Y/8)
   rrca           ;; [1] / rA' = rA / 4 = 2*int(Y/8)
   rrca           ;; [1] \ 
   ;; rL' = rL + rA' = 10 * int(Y/8)
   add   a, l     ;; [1] / rL = rL + rA' = 8*int(Y/8) + 2*int(Y/8) = 10*int(Y/8)
   ld    l, a     ;; [1] \ 

   ;; Now rHL = 256*L + 10*R
   ;; Just multiply rHL' = 8*rHL = 8*(256*L + 10*R) = 2048*L + 80*R
   add   hl, hl   ;; [3] / rHL' = 8*rHL
   add   hl, hl   ;; [3] | rHL' = 2048*L + 80*R
   add   hl, hl   ;; [3] \ 

   ;; To complete the calculations, we only need to add the X-Coordinate
   ;; and the pointer to the start of the video buffer
   
   ;; Add up X coordinate
   ld     b, #0   ;; [2] / As rC = X-Coordinate, having rB=0 makes rBC = X-Coordinate
   add   hl, bc   ;; [3] \ rHL' = rHL + X

   ;; Add up screen start address we still keep in DE
   add   hl, de   ;; [3] rHL' = rHL + screen_start

   ;; HL now contains the pointer to the byte in the video buffer. Just return it
   ret            ;; [3] return rHL = Pointer to the video buffer at (X,Y) byte coordinates
