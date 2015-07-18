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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_px2byteM1
;;
;;    Transforms 4 pixel colour values [0-3] into a byte value in the video memory
;; pixel format for Mode 1.
;;
;; C Definition:
;;    <u8> <cpct_px2byteM1> (<u8> *px0*, <u8> *px1*, <u8> *px2*, <u8> *px3*);
;;
;; Input Parameters (2 Bytes):
;;    (1B _) px0 - Firmware colour value for left         pixel (pixel 0) [0-3]
;;    (1B _) px1 - Firmware colour value for center-left  pixel (pixel 1) [0-3]
;;    (1B _) px2 - Firmware colour value for center-right pixel (pixel 2) [0-3]
;;    (1B _) px3 - Firmware colour value for right        pixel (pixel 3) [0-3]
;;
;; Returns:
;;    u8 - byte with *px0*, *px1*, *px2* and *px3* colour information in screen pixel format.
;;
;; Assembly call:
;;    This function does not have assembly entry point. You should use C entry
;; point and put parameters on the stack, this way:
;;    > ld   bc, #0x0103      ;; B = *px1* = 1, C = *px0* = 3 (Firmware colours)
;;    > ld   de, #0x0200      ;; D = *px3* = 2, E = *px2* = 0 (Firmware colours)
;;    > push de               ;; Put parameters on the stack (in reverse order, always)
;;    > push bc               ;; 
;;    > call _cpct_px2byteM1  ;; Call the function on the C entry point
;;    > pop  bc               ;; Recover parameters from stack to leave it at its previous state
;;    > pop  de               ;; 
;;
;; Parameter Restrictions:
;;    * *px0*, *px1*, *px2* and *px3* must be firmware colour values in the range 
;; [0-3]. If any of them is greater than 3, unexpected colours may appear on screen.
;; *px?* are used as indexes in a colour conversion table, and values greater
;; than 3 would point outside the table, getting random memory values, which will
;; lead to random colours on screen.
;;
;; Details:
;;    Converts 4 firmware colour values for 4 consecutive pixels into a byte value
;; in the video memory pixel format for Mode 1. This video memory pixel format 
;; is the way pixel colour values are encoded in video memory. Concretely, in 
;; Mode 1, each byte contains 4 consecutive pixels formatted as follows:
;; (start code)
;;  ___________________________________________________________________________________
;;                        <----------- 1 byte ----------->
;;  Screen         => [...[pixelA][pixelB][pixelC][pixelD]...] (4 pixels, consecutive)
;;  ===================================================================================
;;  Video Memory   => [...[  A   B  C  D   A  B  C   D   ]...] (1  byte, 8 bits)
;;  Pixel A   (10) => [...[  0   ·  ·  ·   1  ·  ·   ·   ]...] (2  bits)
;;  Pixel B   (32) => [...[  ·   2  ·  ·   ·  3  ·   ·   ]...] (2  bits)
;;  Pixel C   (54) => [...[  ·   ·  4  ·   ·  ·  5   ·   ]...] (2  bits)
;;  Pixel D   (76) => [...[  ·   ·  ·  6   ·  ·  ·   7   ]...] (2  bits)
;;  -----------------------------------------------------------------------------------
;;              Scheme 1. Screen pixel format and video memory
;; (end)
;;   
;;    This function uses a 4-byte conversion table to get screen formatted values
;; for each one of the two pixels given. These formatted values are then OR'ed to 
;; get the final byte that you may use to draw on screen.
;;
;; Destroyed Register values:
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    29 bytes (25 bytes code, 4 bytes colour conversion table)
;;
;;   Note - Colour conversion table is shared with <cpct_drawCharM1>. If you use both
;; functions, only one copy of the colour table is loaded into memory.
;;
;; Time Measures:
;; (start code)
;; Case  | Cycles | microSecs (us)
;; ---------------------------------
;; Any   |  341   |  85,25 
;; (end code)
;;    NC=Number of colours to convert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Pixel colour table defined in cpct_drawCharM1
.globl dc_mode1_ct

_cpct_px2byteM1::
   ;; Point HL to the start of the first parameter in the stack
   ld   hl, #2           ;; [10] HL Points to SP+2 (first 2 bytes are return address)
   ld    c, h            ;; [ 4] C = 0 (Byte in video memory pixel format)
   add  hl, sp           ;; [11]    , to use it for getting parameters from stack

   ;;
   ;; Transform next pixel into Screen Pixel Format
   ;;
   ld    b, #4           ;; [ 7] We have 4 pixels to mix into 1 byte, so set loop counter to 4
px1_repeat:
   ld   de, #dc_mode1_ct ;; [10] DE points to the start of the colour table
   ld    a, (hl)         ;; [ 7] A = Firmware colour for next pixel (to be added to DE, 
                         ;; .... as it is the index of the colour value to retrieve)
   ;; Compute DE += Pixel 0 (A)
   add   e               ;; [ 4] | E += A
   ld    e, a            ;; [ 4] |
   sub   a               ;; [ 4] A = 0 (preserving Carry Flag)
   adc   d               ;; [ 4] | D += Carry
   ld    d, a            ;; [ 4] |

   ld    a, (de)         ;; [ 7] A = Screen format for Firmware colour for Pixel
   or    c               ;; [ 4] Mix (OR) pixel format with accumulated previous pixel format conversions
   rlca                  ;; [ 4] Rotate A left, to left space for next pixel at the same 2 bits (7 and 3)
   ld    c, a            ;; [ 4] B = Acumulated screen pixels format
   inc  hl               ;; [ 6] HL points to next pixel in firmware colour (next parameter)
   djnz px1_repeat       ;; [13/8] Repeat until B=0 (until 4 pixels have been converted)

   ld    l, c            ;; [ 4] L = B, put return value into L

   ret                   ;; [10] return
