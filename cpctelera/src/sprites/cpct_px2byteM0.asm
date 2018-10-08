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
;; Function: cpct_px2byteM0
;;
;;    Transforms 2 pixel colour values [0-15] into a byte value in the video memory
;; pixel format for Mode 0.
;;
;; C Definition:
;;    <u8> <cpct_px2byteM0> (<u8> *px0*, <u8> *px1*);
;;
;; Input Parameters (2 Bytes):
;;    (1B H) px0 - Firmware colour value for left  pixel (pixel 0) [0-15]
;;    (1B L) px1 - Firmware colour value for right pixel (pixel 1) [0-15]
;;
;; Returns:
;;    u8 - byte with *px0* and *px1* colour information in screen pixel format.
;;
;; Assembly call:
;;    > call cpct_px2byteM0_asm
;;
;; Parameter Restrictions:
;;    * *px0* and *px1* must be firmware colour values in the range [0-15]. If
;; any of them is greater than 15, unexpected colours may appear on screen. *px0*
;; and *px1* are used as indexes in a colour conversion table, and values greater
;; than 15 would point outside the table, getting random memory values, which will
;; lead to random colours on screen.
;;
;; Details:
;;    Converts 2 firmware colour values for 2 consecutive pixels into a byte value
;; in the video memory pixel format for Mode 0. This video memory pixel format 
;; is the way pixel colour values are encoded in video memory. Concretely, in 
;; Mode 0, each byte contains 2 consecutive pixels formatted as follows:
;; (start code)
;;  ______________________________________________________________________
;;                        <---- 1  byte ---->
;;  Screen         => [...[pixelX ][ pixelY ]...] (2 pixels, consecutive)
;;  ======================================================================
;;  Video Memory   => [...[ X Y X Y X Y X Y ]...] (1  byte, 8 bits)
;;  Pixel X (3210) => [...[ 0 · 2 · 1 · 3 · ]...] (4  bits)
;;  Pixel Y (dcba) => [...[ · a · c · b · d ]...] (4  bits)
;;  ----------------------------------------------------------------------
;;        Scheme 1. Screen pixel format and video memory
;; (end)
;;   
;;    This function uses a 16-byte conversion table to get screen formatted values
;; for each one of the two pixels given. These formatted values are then OR'ed to 
;; get the final byte that you may use to draw on screen.
;;
;; Destroyed Register values:
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings  - 28 bytes 
;;   ASM-bindings -  26 bytes 
;;    dc_mode0_ct - +16 bytes Color conversion table
;;
;;   Note - Colour conversion table is shared with <cpct_drawCharM0>. If you use both
;; functions, only one copy of the colour table is loaded into memory.
;;
;; Time Measures:
;; (start code)
;; Case       | microSecs (us) |  CPU Cycles
;; ------------------------------------------
;; Any        |      39        |     156
;; ------------------------------------------
;; ASM-saving |      -9        |     -36
;; ------------------------------------------
;; (end code)
;;    NC=Number of colours to convert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Pixel colour table defined in cpct_drawCharM0
.globl dc_mode0_ct
.include "macros/cpct_maths.h.s"

   ;; Convert first parameter (Pixel 0) to screen pixel format
   ld   a, l             ;; [1] A = L (A = First parameter, pixel 0, palette index)
   ld  de, #dc_mode0_ct  ;; [3] DE points to conversion table (dc_mode0_ct)
   add_de_a              ;; [5] DE = DE + A
   ld   a, (de)          ;; [2] A = *(DE + A) => A = Pixel 0 converted to screen pixel format
   sla  a                ;; [2] A <<= 1, as Screen formats in table are in Pixel Y disposition 
   ld   b, a             ;; [1] B = Pixel 0 in screen format

   ;; Convert second parameter (Pixel 1) to screen pixel format
   ld   a, h             ;; [1] A = H (A = Second parameter, pixel 1, palette index)
   ld  de, #dc_mode0_ct  ;; [3] DE points to conversion table (dc_mode0_ct)
   add_de_a              ;; [5] DE = DE + A
   ld   a, (de)          ;; [2] A = *(DE + A) => A = Pixel 1 converted to screen pixel format

   ;; Mix both pixels in a single byte and return
   or   b                ;; [1] A = A or B (Mix Pixel 0 and Pixel 1 screen pixel format into A)
   ld   l, a             ;; [1] L = A (Return value: byte with pixels 0 and 1 in screen pixel format)
   ret                   ;; [3] Return
