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
;; Function: cpct_px2byteM0
;;
;;    Transforms 2 pixel colour values [0-15] into a byte value in the video memory
;; pixel format for Mode 0.
;;
;; C Definition:
;;    <u8> <cpct_px2byteM0> (<u8> *px0*, <u8> *px1*);
;;
;; Input Parameters (2 Bytes):
;;    (1B _) px0 - Firmware colour value for left  pixel (pixel 0) [0-15]
;;    (1B _) px1 - Firmware colour value for right pixel (pixel 1) [0-15]
;;
;; Returns:
;;    u8 - byte with *px0* and *px1* colour information in screen pixel format.
;;
;; Assembly call:
;;    This function does not have assembly entry point. You should use C entry
;; point and put parameters on the stack, this way:
;;    > ld   bc, #0x0103      ;; B = *px1* = 1, C = *px0* = 3 (Firmware colours)
;;    > push bc               ;; Put parameters on the stack
;;    > call _cpct_px2byteM0  ;; Call the function on the C entry point
;;    > pop  bc               ;; Recover parameter from stack to leave it at its previous state
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
;;    48 bytes (32 bytes code, 16 bytes colour conversion table)
;;
;;   Note - Colour conversion table is shared with <cpct_drawCharM0>. If you use both
;; functions, only one copy of the colour table is loaded into memory.
;;
;; Time Measures:
;; (start code)
;; Case  | Cycles | microSecs (us)
;; ---------------------------------
;; Any   |  145   |  36,25 
;; (end code)
;;    NC=Number of colours to convert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Pixel colour table defined in cpct_drawCharM0
.globl dc_mode0_ct

_cpct_px2byteM0::
   ;; Point HL to the start of the first parameter in the stack
   ld  hl, #2            ;; [10] HL Points to SP+2 (first 2 bytes are return address)
   add hl, sp            ;; [11]    , to use it for getting parameters from stack

   ;;
   ;; Transform pixel 0 into Screen Pixel Format
   ;;
   ld   de, #dc_mode0_ct ;; [10] DE points to the start of the colour table
   ld    a, (hl)         ;; [ 7] A = Firmware colour for pixel 0 (to be added to DE, 
                         ;; .... as it is the index of the colour value to retrieve)
   ;; Compute DE += Pixel 0 (A)
   add   e               ;; [ 4] | E += A
   ld    e, a            ;; [ 4] |
   sub   a               ;; [ 4] A = 0 (preserving Carry Flag)
   adc   d               ;; [ 4] | D += Carry
   ld    d, a            ;; [ 4] |

   ld    a, (de)         ;; [ 7] A = Screen format for Firmware colour of Pixel 0
   sla   a               ;; [ 8] A <<= 1, as Screen formats in table are in Pixel Y disposition 
                         ;; .... (see Scheme 1 in this function's documentation)
   ld    b, a            ;; [ 4] B = Transformed value for pixel 0

   ;;
   ;; Transform pixel 1 into Screen Pixel Format
   ;;
   ld   de, #dc_mode0_ct ;; [10] DE points to the start of the colour table
   inc  hl               ;; [ 6] HL points to next parameter (Pixel 1)
   ld    a, (hl)         ;; [ 7] A = Firmware colour for pixel 1 (to be added to DE, 
                         ;; .... as it is the index of the colour value to retrieve)
   ;; Compute DE += Pixel 1 (A)
   add   e               ;; [ 4] | E += A
   ld    e, a            ;; [ 4] |
   sub   a               ;; [ 4] A = 0 (preserving Carry Flag)
   adc   d               ;; [ 4] | D += Carry
   ld    d, a            ;; [ 4] \
   ld    a, (de)         ;; [ 7] A = Screen format for Firmware colour of Pixel 0

   ;; Merge both values and return result
   or    b               ;; [ 4] A = Merged value of transformed pixel values (px1 | px2)
   ld    l, a            ;; [ 4] L = A, put return value into L

   ret                   ;; [10] return
