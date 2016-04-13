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

;;
;; Title: cpct_getRandom_glfsr16
;;    Pseudo-random number generator that uses a 16-bits Galois Linear-Feedback Shift Register
;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_getRandom_glfsr16_u8
;;
;;    Return a pseudo-random byte using Galois Linear-Feedback Shift 
;; Register (G-LFSR) method, with a 16-bits state register.
;;
;; C Definition:
;;    <u8>  <cpct_getRandom_glfsr16_u8>  ();
;;
;; Assembly call (Input parameter on L):
;;    > call cpct_getRandom_glfsr16_u8_asm
;;
;; Return value (Assembly calls, return L=A=random 8-bits, HL=random 16-bits):
;;    <u8> - Next 8-bits pseudo-random value.
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
;; Details:
;;      This function implements a Galois Linear-Feedback Shift Register with a 16
;; bits state. This Shift Register does produce 65535 16-bits values without repetition,
;; then cycles again. That is, it is like having the 65536 (excluding 0) possible 16-bits
;; values ordered in a pseudo-random manner, and getting each one at a time. 
;;
;;      The implementation of the Shift Register is based on some specific 16-bits
;; values denominated TAPS (Check <GLFSR16_TAPSET>). These values are bit-masks that
;; select some bits to implement a linear polynomial shifting operation. Depending
;; on the bits selected in the bit-mask, the polynomial implemented changes. There
;; exists 1024 different polynomials that mathematically ensure a 65535-value traversal
;; without repetition. Any other polynomial will have a shorter period. These
;; 1024 TAPSETs that implement the polynomials with full 16-bits traversals are 
;; defined in <GLFSR16_TAPSET> enumeration. Use <cpct_setTaps_glfsr16> with any of the 
;; these tapsets to select your desired traversal.
;;
;; Use example:
;;    Basic use of this function does not require any kind of set-up, just calling
;; the function each time you want a pseudo-random number
;; (start code)
;;    // Get next pseudo-random 8-bits number
;;    u8 getRandomNumber() {
;;       return cpct_getRandom_glfsr16_u8();
;;    }
;; (end code)
;;    However, this will return always the same sequence, in the same order. You might
;; consider setting the staring seed at some initialization point,
;; (start code)
;;    // Count loops until user presses a key
;;    u16 countUntilUserPressesAKey() {
;;       u16 loops;
;;       do {
;;          loops++;
;;          cpct_scanKeyboard();
;;       } while ( !cpct_isAnyKeyPressed() );
;;
;;       return loops;
;;    }
;;
;;    // Initialization code (set initial seed)
;;    u16 loops = countUntilUserPressesAKey();
;;    cpct_setSeed_glfsr16(loops);
;; (end code)
;;    This code will set the starting state of the internal shift register to a value 
;; depending on when the user has pressed a key, which will randomize the starting 
;; point in the 65535-values sequence that the pseudo-random generator produces.
;; However, take into account that the 65535-value traversal will be in the same
;; order, unless you changed it calling <cpct_setTaps_glfsr16>.
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
;; Credits and References:
;;    This function is based on Galois Linear-Feedback Shift Register method.
;;  - On Linear-Feedback Shift Registers: https://es.wikipedia.org/wiki/LFSR
;;  - Theory on LSFRs: http://web.archive.org/web/20010605005727/homepage.mac.com/afj/lfsr.html
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_getRandom_glfsr16_u16
;;
;;    Return a pseudo-random 16-bits value using Galois Linear-Feedback Shift 
;; Register (G-LFSR) method, with a 16-bits state register.
;;
;; C Definition:
;;    <u16> <cpct_getRandom_glfsr16_u16> ();
;;
;; Assembly call (Input parameter on L):
;;    > call cpct_getRandom_glfsr16_u16_asm
;;
;; Return value (Assembly calls, return HL=random 16-bits):
;;    <u16> - Next 16-bits pseudo-random value.
;;
;; Consult for more information:
;;    <cpct_getRandom_glfsr16_u8>, which is the same function.
;;
;; Details:
;;    This function is exactly the same as <cpct_getRandom_glfsr16_u8>. Both 
;; functions share the 100% of their code. The only difference is the return
;; type: <cpct_getRandom_glfsr16_u8> returns a <u8> (in L register) and 
;; <cpct_getRandom_glfsr16_u16> returns a <u16> (in HL register). So, from
;; C perspective, same code is called, and 16 bits are returned, but sometimes
;; only 8 bits are used (when <cpct_getRandom_glfsr16_u8>). From assembly 
;; point of view, you call the same function, and may decide to use return in
;; L (8 bits) or in HL (16 bits) at your will.
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
