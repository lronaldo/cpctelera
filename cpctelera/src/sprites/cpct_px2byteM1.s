;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2023 Joaquin Ferrero ( https://github.com/joaquinferrero )
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
;;
;; Details:
;;    Converts 4 firmware colour values for 4 consecutive pixels into a byte value
;; in the video memory pixel format for Mode 1. This video memory pixel format 
;; is the way pixel colour values are encoded in video memory. Concretely, in 
;; Mode 1, each byte contains 4 consecutive pixels formatted as follows:
;; (start code)
;;  _____________________________________________________________________________
;;  Screen
;;  Layout       => [<------------ 1 byte ------------>]  (in byte boundary)
;;  pixel format => (pixel A)(pixel B)(pixel C)(pixel D)  (4 pixels, consecutive)
;;  =============================================================================
;;  Video memory
;;  Byte layout       => [ 7654 3210 ]  (1 byte, 8 bits order)
;;  M1 pixels encoded => [ ABCD ABCD ]  (4 pixels bits order)
;;  pixel A    [ 10 ] => [ 0... 1... ]  (2 low bits of pixel colour value)
;;  pixel B    [ 10 ] => [ .0.. .1.. ]  (2 low bits of pixel colour value)
;;  pixel C    [ 10 ] => [ ..0. ..1. ]  (2 low bits of pixel colour value)
;;  pixel D    [ 10 ] => [ ...0 ...1 ]  (2 low bits of pixel colour value)
;;  -----------------------------------------------------------------------------
;;              Scheme 1. Screen pixel format and video memory
;; (end)
;;   
;; Destroyed Register values:
;;    AF, BC, HL
;;
;; Required memory:
;;    33 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | microSecs (us) | CPU Cycles
;; ------------------------------------
;; Any   |      57        |   201
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cpct_px2byteM1_asm::
_cpct_px2byteM1::
   ;; Point HL to the start of the first parameter in the stack
   ld   hl, #2           ;; [3] HL Points to SP+2 (first 2 bytes are return address)
   ld    b, l            ;; [1] B = L (2) counter to loop twice
   add  hl, sp           ;; [3]    , to use it for getting parameters from stack
   ld    c, #0x10        ;; [2] C = 0x10 Mask to set into high nibble

   ;; Process the first pixel
   ld    a, (hl)         ;; [2] A = Firmware colour first pixel
   rra                   ;; [1] Shift to the right, the bit 0 (low bit of pixel) is stored into fC
   jr   nc, loop1        ;; [2/3] If fC is set...
   or    c               ;; [1] ... set the low bit into high nibble
loop1:

   ;; Loop for second and third pixels
px1_repeat:
   add   a               ;; [1] Make space to the next pixel,
   add   a               ;; [1] ... shifting two bits to the left
   inc  hl               ;; [2] HL points to next pixel in firmware colour (next parameter)
   or  (hl)              ;; [2] Read the pixel

   rra                   ;; [1] Shift to the right, the bit 0 (low bit of pixel) is stored into fC
   jr   nc, loop2        ;; [2/3] If fC is set...
   or    c               ;; [1] ... set the low bit into high nibble
loop2:
   djnz px1_repeat       ;; [3/4] Repeat until B=0 (until 2 pixels have been converted)

   ;; Process the last pixel
   add   a               ;; [1] Shift to the left, to left space for last pixel
   inc  hl               ;; [2] HL points to next pixel in firmware colour (next parameter)
   ld    b, (hl)         ;; [2] A = Firmware colour for next pixel
   srl   b               ;; [2] Shift to the right, the bit 0 (low bit of pixel) is stored into fC
   jr   nc, loop3        ;; [2/3] If fC is set...
   or    c               ;; [1] ... set the low bit into high nibble
loop3:
   or    b               ;; [1] Set the high bit into high nibble

   ld    l, a            ;; [1] L = A, put return value into L

   ret                   ;; [3] return

