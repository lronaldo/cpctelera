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
.module cpct_video
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_getScreenPtr
;;
;;    Returns a byte-pointer to a screen memory location, given its X, Y coordinates.
;;
;; C Definition:
;;    <u8>* <cpct_getScreenPtr> (void* *screen_start*, <u8> *x*, <u8> *y*)
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) screen_start - Pointer to the start of the screen (or a backbuffer)
;;    (1B C ) x            - [0-79]  Byte-aligned column starting from 0 (x coordinate, 
;;                           *in bytes*)
;;    (1B B ) y            - [0-199] row starting from 0 (y coordinate, *in pixels*)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_getScreenPtr_asm
;;
;; Parameter Restrictions:
;;    * *screen_start* will normally be the start of the video memory where you want to 
;; draw something. However, you may give a pointer to the start of a screen backbuffer
;; located everywhere in memory.
;;    * *x* must be in the range [0-79]. Screen is always 80 bytes wide (unless you
;; change CRTC registers), and this function is byte-aligned, so you have to give it
;; a byte coordinate (*NOT a pixel one!*). 
;;    * *y* must be in the range [0-199]. Screen is always 200 pixels high (again, 
;; unless you change CRTC registers). Pixels and bytes always coincide in vertical
;; resolution, so this coordinate is the same in bytes that in pixels.
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
;;    What this function exactly does is this calculation,
;;    > return screen_start + 80 * ((int)(y / 8)) + 2048 * (y % 8) + x
;;
;;    This calculation is based on the default layout of the screen, which is
;; 80 bytes wide (80 bytes per pixel line) and 200 pixel lines high. Pixel
;; lines are grouped into 25 groups of 8 pixel lines, as it is explained in
;; <cpct_drawSprite>. Each group of 8 pixel lines starts 0x50 bytes away 
;; from the previous one (0x50 = 80, hence the term [80 * ((int)(y / 8))]) and
;; the each lines inside a group is separated by 0x800 bytes from its
;; predecessor (0x800 = 2048, and we have the other term [2048 * (y % 8)]). 
;;
;;    All the previous paragraph explains how much we have to displace from
;; *screen_start* to arrive to the start of the *y* pixel line. Once we are
;; there, the byte *x* is just *x* bytes away, so we only have to add *x*
;; to the formula, and we are done.
;;
;; Destroyed Register values:
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 26 bytes
;;  ASM-bindings - 22 bytes
;;
;; Time Measures: 
;; (start code)
;;     Case   | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any    |      34        |     126
;; -----------------------------------------
;; Asm saving |     -13        |     -41
;; -----------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; input
;;   reg B  := Y [0-199]
;;   reg C  := X [0-79]
;;   reg DE := screen_start [$0000-$FFFF] (&C000 typical)
;;
;; screen layout
;;   lines := [0-24]
;;   rows  := [0-7]
;;
;;   1 lines == 8 rows, so Y is %[L|L|L|L|L|R|R|R]
;;       rows  := Y % 8
;;       lines := Y / 8
;;
;; output
;;   reg HL := byte pointer into screen memory
;;

    ;; Extract rows from Y coordinate
    ld   a, b		;;[1B  4T  1u]; H = B & $07
    and  %00000111	;;[2B  7T  2u]; |
    ld   h, a		;;[1B  4T  1u]; | H := Y % 8 = rows

    ;; Extract lines from Y coordinate
    ld   a, b		;;[1B  4T  1u]; L = B & $F8
    and  %11111000	;;[2B  7T  2u]; |
    ld   l, a		;;[1B  4T  1u]; | L := 8 * int(Y/8) = 8 * lines

    ;; Calculate 8*(256*rows+10*lines) = 2048*rows+80*lines
    srl  a		;;[2B  8T  2u]; A = L / 4
    srl  a		;;[2B  8T  2u]; | A := 2 * lines

    add  a, l		;;[1B  4T  1u]; L = A + L
    ld   l, a		;;[1B  4T  1u]; | L := (2+8) * lines = 10 * lines

    add  hl, hl		;;[1B 11T  3u]; HL = HL * 8
    add  hl, hl		;;[1B 11T  3u]; |
    add  hl, hl		;;[1B 11T  3u]; | HL := 2048 * rows + 80 * lines

    ;; Add X coordinate
    ld   b, 0		;;[2B  7T  2u]; BC = X
    add  hl, bc		;;[1B 11T  3u]; | HL := HL + X

    ;; Add screen start address
    add  hl, de		;;[1B 11T  3u]; HL := HL + screen_start

    ;; Return
    ret			;;[1B 10T  3u]

;; Total: 22 Bytes, 126 CPU cycles, 34 us

