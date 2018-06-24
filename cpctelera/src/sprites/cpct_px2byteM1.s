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
;; Function: cpct_px2byteM1
;;
;;    Transforms 4 pixel colour values [0-3] into a byte value in the video memory
;; pixel format for Mode 1.
;;
;; C Definition:
;;    <u8> <cpct_px2byteM1> (<u8> *px0*, <u8> *px1*, <u8> *px2*, <u8> *px3*);
;;
;; Input Parameters (4 Bytes - All received in the stack):
;;    (1B) px0 - Firmware colour value for left         pixel (pixel 0) [0-3]
;;    (1B) px1 - Firmware colour value for center-left  pixel (pixel 1) [0-3]
;;    (1B) px2 - Firmware colour value for center-right pixel (pixel 2) [0-3]
;;    (1B) px3 - Firmware colour value for right        pixel (pixel 3) [0-3]
;;
;; Returns:
;;    u8 - byte with *px0*, *px1*, *px2* and *px3* colour information in screen pixel format.
;;
;; Assembly call:
;;    All parameters must be passed on the stack and recovered 
;; afterwards. This is an example call:
;;    > ld   bc, #0x0103         ;; B = *px1* = 1, C = *px0* = 3 (Firmware colours)
;;    > ld   de, #0x0200         ;; D = *px3* = 2, E = *px2* = 0 (Firmware colours)
;;    > push de                  ;; Put parameters on the stack (in reverse order, always)
;;    > push bc                  ;; 
;;    > call cpct_px2byteM1_asm  ;; Call the function on the C entry point
;;    > pop  bc                  ;; Recover parameters from stack to leave it at its previous state
;;    > pop  de                  ;; 
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
;; Case  | microSecs (us) | CPU Cycles
;; ------------------------------------
;; Any   |      96        |   384
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Include required macro files
.include "macros/cpct_maths.h.s"

cpct_px2byteM1_asm::
_cpct_px2byteM1::
   ;; Point HL to the start of the first parameter in the stack
   ld   hl, #2           ;; [3] HL Points to SP+2 (first 2 bytes are return address)
   ld    c, h            ;; [1] C = 0 (Byte in video memory pixel format)
   add  hl, sp           ;; [3]    , to use it for getting parameters from stack

   ;;
   ;; Transform next pixel into Screen Pixel Format
   ;;
   ld    b, #4           ;; [2] We have 4 pixels to mix into 1 byte, so set loop counter to 4
px1_repeat:
   ld   de, #pen2mode1px ;; [3] DE points to the start of the colour table
   ld    a, (hl)         ;; [2] A = Firmware colour for next pixel (to be added to DE, 
                         ;; .... as it is the index of the colour value to retrieve)
   add_de_a              ;; [5] DE += A (A contains next color translation to Pixel 0 bitpattern)
   
   ld    a, (de)         ;; [2] A = Screen format for Firmware colour for Pixel
   or    c               ;; [1] Mix (OR) pixel format with accumulated previous pixel format conversions
   rlca                  ;; [1] Rotate A left, to left space for next pixel at the same 2 bits (7 and 3)
   ld    c, a            ;; [1] C = Acumulated screen pixels format
   inc  hl               ;; [2] HL points to next pixel in firmware colour (next parameter)
   djnz px1_repeat       ;; [3/4] Repeat until B=0 (until 4 pixels have been converted)

   ld    l, c            ;; [1] L = B, put return value into L

   ret                   ;; [3] return

;;
;;    Mode 1 Color conversion table (PEN to Screen pixel format)
;;
;;    This table converts PEN values (palette indexes from 0 to 4) into screen pixel format values in mode 1. 
;; In mode 1, each byte has 4 pixels (P0, P1, P2, P3). This table converts to Pixel 0 (P0) format. Getting values for
;; other pixels require shifting this byte to the right 1 to 3 times (depending on which pixel is required).
;;
pen2mode1px: .db 0x00, 0x08, 0x80, 0x88