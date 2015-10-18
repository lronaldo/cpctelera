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
.module cpct_random

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setTaps_glfsr16
;;
;;    Returns a pseudo-random byte uniformly distributed using fast method (33*Seed mod 257)
;;
;; C Definition:
;;    <u8> <cpct_setTaps_glfsr16> (<u16> *tapset*) __z88dk_fastcall;
;;
;; Input Parameters (2 bytes):
;;    (2B HL) tapset - set of tap bits that <cpct_getRandomu8_glfsr16> will use to produce
;; pseudo-random numbers. 
;;
;; Assembly call:
;;    > call cpct_setTaps_glfsr16_asm
;;
;; Parameter Restrictions:
;;    * *tapset* could theoretically be any 16-value. However, there are only 1024 values that
;; will produce complete sequences of 65535 16-bit pseudo-random numbers without repetition. For 
;; those sequences, a set of 1024 macros <GLFSR16_TAPSET_YYYY> (with YYYY from 0000 to 1023)
;; are defined. If any of this macros is used as *tapset*, a sequence of 65535 16-bit pseudo-random
;; numbers without repetition is guaranteed. With any other value, shorter sequences will be 
;; produced, with no guarantee on their final frequency.
;;
;; Known limitations:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;
;; Important details:
;;    * TODO
;;
;; Details:
;;    * TODO
;;
;; Destroyed Register values: 
;;      A
;;
;; Required memory:
;;      9 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |      13        |    52
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_randUnif_glfsr16_hightaps
.globl cpct_randUnif_glfsr16_lowtaps

_cpct_setTaps_glfsr16::
cpct_setTaps_glfsr16_asm::
   ;; HL holds parameter, as this function is __z88dk_fastcall
   ld  a, h                                  ;; [1] A = High taps
   ld  (cpct_randUnif_glfsr16_hightaps+1), a ;; [4] Set high value for taps (to be XORred with H)
   ld  a, l                                  ;; [1] A = Low taps
   ld  (cpct_randUnif_glfsr16_lowtaps+1), a  ;; [4] Set low value for taps (to be XORred with L)
   ret                                       ;; [3] Return
