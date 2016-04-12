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
;; Function: cpct_getRandom_glfsr16_u8
;; Function: cpct_getRandom_glfsr16_u16
;;
;;    Return a pseudo-random byte using Galois Linear-Feedback Shift 
;; Register (G-LFSR) method, with a 16-bits state register.
;;
;; C Definition:
;;    <u8>  <cpct_getRandomu8_glfsr16>  ();
;;    <u16> <cpct_getRandomu16_glfsr16> ();
;;
;; Assembly call (Input parameter on L):
;;    > call cpct_getRandom_glfsr16_u8_asm
;;    > call cpct_getRandom_glfsr16_u16_asm
;;
;; Known limitations:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;    * Seed (machine state) must *never* be zero. 
;;    * Returned value using all 16 bits (<u16>) will never be 0, so it is
;; advisable to use returned value-1 as final random number. This limitation does 
;; not affect <u8> return values.
;;    * Although the whole sequence has a period of 65535 numbers without repetition,
;; the way numbers are traversed is quite predictable. This function is not recommended
;; when good quality random numbers are required. However, a combination with other
;; functions may yield much better results.
;;
;; Important details:
;;      TODO
;;
;; Details:
;;      TODO
;;
;; Destroyed Register values: 
;;      AF, HL
;;
;; Required memory:
;;      22 bytes
;;
;; Time Measures:
;; (start code)
;;   Case | microSecs(us) | CPU Cycles
;; -------------------------------------
;;  Best  |      19       |     76
;; -------------------------------------
;;  Worst |      26       |    104
;; -------------------------------------
;; (end code)
;;
;; Credits:
;;    This function is based on Galois Linear-Feedback Shift Register method.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; u16 and u8 functions do the same: 
;;   one will only consider L as return value whereas the other will use HL
_cpct_getRandom_glfsr16_u8::
cpct_getRandom_glfsr16_u8_asm::

_cpct_getRandom_glfsr16_u16::
cpct_getRandom_glfsr16_u16_asm::

cpct_randUnif_glfsr16_seed == .+1
   ld  hl, #0xABCD  ;; [3] DE = Machine state (seed)

   ;; Calculate the Galois Linear-Feedback Shift Register Function
   ;; which gives a period length of 65535. The way the sequence is
   ;; traversed is determined by HL parameter, which has a mask 
   ;; with the 'taps' used in the sequence. Default function 
   ;; is nextbit = x^16 + x^15 + x^14 + x^13 + x^11 + x^10 + x^9 + x^8 + x^7 + x^6 + x^5 + x^4 + x^2 + x + 1
   xor  a           ;; [1] Reset carry flag
   rr   h           ;; [2] | Rotate HL to the right, inserting a 0 as the leftmost H bit (carry flag = 0)
   rr   l           ;; [2] |
   jr  nc, end      ;; [2/3] Carry hold previous rightmost bit. We do XOR operations only if it was a 1.

   ;; Apply XOR operations to mix 16-bits status seed with TAPS (the bits
   ;; that implement the Linear-Feedback Shifting method)
   ld   a, h        ;; [1] | Apply sequence formulae XORing taps marked by HL (which is a mask)

cpct_randUnif_glfsr16_hightaps == .+1
   xor  #0xF7       ;; [2] | HL = HL xor TAPS 
   ld   h, a        ;; [1] |  (Bits to be changed according to Galois LFSR for ...
   ld   a, l        ;; [1] |   maximum sequence: period 2^16)

cpct_randUnif_glfsr16_lowtaps == .+1
   xor  #0xFB       ;; [2] |
   ld   l, a        ;; [1] |

end:
   ;; Save new machine state and return
   ld (cpct_randUnif_glfsr16_seed), hl ;; [5] Saves the new state into its place

   ret              ;; [3] Return
