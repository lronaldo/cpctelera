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
.module cpct_video
  
.include /videomode.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_getScreenPtr
;;
;;    Returns a byte-pointer to a screen memory location, given its X, Y coordinates.
;; (in *bytes*, NOT in pixels!)
;;
;; C Definition:
;;    <u8*> <cpct_getScreenPtr> (void* *screen_start*, <u8> *x*, <u8> *y*)
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) screen_start - Pointer to the start of the screen (or a backbuffer)
;;    (1B B ) y            - [0-199] row starting from 0 (y coordinate)
;;    (1B C ) x            - [0-79]  Byte-aligned column starting from 0 (x coordinate, 
;; *in bytes*)
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
;;    <u8*> - Pointer to the (*x*,*y*) byte in the screen memory that starts 
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
;;    What this function exactly does is this calculation:
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
;;    35 bytes
;;
;; Time Measures:
;; (start code)
;;     Case   | Cycles  | microSecs (us)
;; ---------------------------------------
;;     Any    |  224    |   56.00
;; ---------------------------------------
;; Asm saving |  -63    |  -15.75
;; ---------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_getScreenPtr::
   ;; Get Parameters from stack
   pop  af        ;; [10] AF = Return Address
   pop  de        ;; [10] DE = Pointer to start of screen memory
   pop  bc        ;; [10] B = y coordinate in bytes, C = x coordinate in bytes
   push bc        ;; [11] --- Restore stack status
   push de        ;; [11]
   push af        ;; [11] 

cpct_getScreenPtr_asm::      ;; Assembly entry point
   ;; Add up X coordinate to screen_memory 
   ;; (DE += C, as DE = screen_memory, and C = x coordinate)
   ld    a, e     ;; [ 4] / E = E + x   (As C = x)
   add   c        ;; [ 4] |   ( If E + x > 0xFF, Carry is set. We must Add it to D )
   ld    e, a     ;; [ 4] \
   sub   a        ;; [ 4] A = 0 (preserving carry, and with only 4 cycles)
   ld    h, a     ;; [ 4] H = 0 (We will need it to be 0, and profit now from having a 0 on A)
   adc   d        ;; [ 4] | D = D + Carry 
   ld    d, a     ;; [ 4] |   (If E + x did set carry, this will do D += 1, else D += 0)

   ;; Let y' = [ y / 8 ] = int(y / 8) (Integer division by 8)
   ;; Calculate 80y' = (64 + 16)y' (0x50 * y')
   ld    a, b     ;; [ 4] A = y coordinate (which is stored in b)
   and   #0xF8    ;; [ 7] A = 8y'   ( and 0xF8 is the same as 8 * int(y / 8) = 8y' )
   ld    l, a     ;; [ 4] HL = 8y'  (H = 0, L = 8y')
   ld    a, b     ;; [ 4] A = y coordinate (Save for later use, freeing BC)
   add  hl, hl    ;; [11] HL = 8y' + 8y' = 16y'
   ld    b, h     ;; [ 4] | BC = 16y'
   ld    c, l     ;; [ 4] |
   add  hl, hl    ;; [11] HL = 16y' + 16y' = 32y'
   add  hl, hl    ;; [11] HL = 32y' + 32y' = 64y'
   add  hl, bc    ;; [11] HL = 64y' + 16y' = 80y'

   ;; Calculate 2048 * (y % 8)  (0x800 * (y % 8))
   and   #0x07    ;; [ 7]  A = y % 8  (A had y coordinate)
   rlca           ;; [ 4]  A = 2 (y % 8)
   rlca           ;; [ 4]  A = 4 (y % 8)
   rlca           ;; [ 4]  A = 8 (y % 8)
   ld    b, a     ;; [ 4] | BC = 0x100 * 8 (y % 8)
   ld    c, #0    ;; [ 7] |

   ;; Add up everything and return
   add  hl, bc    ;; [11] HL = 80 * [y / 8] + 2048 * (y % 8) 
   add  hl, de    ;; [11] HL = 80 * [y / 8] + 2048 * (y % 8) + (screen_start + x)

   ret            ;; [10] Return
