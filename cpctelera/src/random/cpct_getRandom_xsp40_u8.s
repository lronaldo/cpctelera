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
.module cpct_random

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_getRandom_xsp40_u8
;;
;;    Generates an 8-bits, high-quality, pseudo-random number with each call. It
;; uses 40 state-bits, giving a period of 962.072.672.512 numbers without repeating.
;;
;; C Definition:
;;    <u8> <cpct_getRandom_xsp40_u8> ();
;;
;; Assembly call:
;;    > call cpct_getRandom_xsp40_u8_asm
;;
;; Return value (Assembly calls, return L=A=random 8-bits):
;;    <u8> - Next 8-bits pseudo-random value.
;;
;; Important details:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function implements a sequence of 40-bits states with period (962.072.672.512). 
;; For each produced state, a random sequence of 8-bits is returned. To do this, 
;; the function has a 40-bits internal state, that is modified each time it is called. 
;; The result is a high-quality pseudo-random number generator that could virtually
;; be considered random for general purposes of Z80-based software.
;;
;;    The sequence calculated by this function is based on a modified version of 
;; <Marsaglia's XORshift+ generator at http://www.jstatsoft.org/article/view/v008i14/xorshift.pdf> 
;; using the tuple (1, 2, 3), with the constant C=255, for an extended matrix T, 
;; composed by 4x4 8-bit shift matrices. This T matrix is populated with A=(I+L^a)*(I+R^b), 
;; B=0, C=(I+R^c), and D=0. Selected tuple does not yield a full-period (which would be 
;; 2^40 - 2^8), but it is very close and is enough for passing all Dieharder tests.
;;
;;    To clarify operations, assuming that the 40-bits state s is composed of 5 8-bits 
;; numbers s=(x, z, y, w, v), this algorithm produces a new state s'=(x',z',y',w',v') 
;; proceeding this way:
;; (start code)
;;   x' = y;
;;   y' = z;
;;   z' = w;
;;   t  = x ^ (x >> 1);
;;   t' = t ^ (t >> 2);
;;   w' = y ^ (y << 3) ^ t';
;;   v' = v + 255;
;;   returned value = w' ^ v' 
;; (end code)
;;
;;   These operations are performed in an optimized fashion. 
;;
;; Destroyed Register values: 
;;      AF, DE, HL
;;
;; Required memory:
;;      37 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |      45        |    180
;; -----------------------------------------
;; (end code)
;;
;; Random quality measures:
;;  * Dieharder tests rank: (113 Pass, 1 Weak, 0 Failed) (340/342 = 99.42%)
;;  * Pseudo-random bit stream velocity: 5,625 us / bit. (8 bits produced in 45 us)
;;  * Pseudo-random bits per second: 177.777,77 bps.
;;
;; Credits:
;;   * Original <XORshift+ algorithm published by George Marsaglia at 
;; http://www.jstatsoft.org/article/view/v008i14/xorshift.pdf>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_getRandom_xsp40_u8::
cpct_getRandom_xsp40_u8_asm::
   ;; First, update Weyl+ sequence seed (v value)
   ld  hl, #_cpct_seed3_xsp40_u8  ;; [3] HL points to V value (seed 3)
   dec (hl)                       ;; [3] V += 255 (same as decrementing 1 in 8-bits arithmetic)
  
   ;; Second, load 32-bit main seeds (xyzw)
   _cpct_seed1_xsp40_u8 == .+1
   _cpct_seed2_xsp40_u8 == .+4
   ld  de, #0x1234                ;; [3] Seed 1: values x, z
   ld  hl, #0x5678                ;; [3] Seed 2: values y, w

   ;; Update x' and z' seed values
   ld  (_cpct_seed1_xsp40_u8),hl  ;; [5] x' = y, z' = w
   
   ;; Calculate y ^ (y << 3)
   ld  a, h        ;; [1] A = y
   add a           ;; [1] |
   add a           ;; [1] |
   add a           ;; [1] | A = y << 3
   xor h           ;; [1] A = y ^ (y << 3)
   ld  h, a        ;; [1] H = y ^ (y << 3)
   
   ;; Calculate t = x ^ (x >> 1)
   ld  a, d        ;; [1] A = x
   rra             ;; [1] A = x >> 1
   xor d           ;; [1] A = x ^ (x >> 1)
   ld  d, a        ;; [1] D = t = x ^ (x >> 1)

   ;; Calculate t' = t ^ (t >> 2)
   ;; and then obtain w' = (y ^ (y << 3)) ^ t'
   rra             ;; [1] |
   srl a           ;; [2] | t = (t >> 2)       (A already contained t)
   xor d           ;; [1] A = t ^ (t >> 2)     (as D contains a copy of t)
   xor h           ;; [1] A = t ^ y ^ (y << 3) (as H contains y ^ (y << 3))
 
   ;; Save new seed values y' and w'
   ld  h, e                       ;; [1] y' = z  (as E contains z value)
   ld  l, a                       ;; [1] w' = A  (as A contains final calculations for w')
   ld  (_cpct_seed2_xsp40_u8), hl ;; [5] save y' = z and w' = A in 2nd seed value

   ;; Calculate return value doing a final mixing XOR
   ;; operation against a Weyl sequence created by rotating v value
   _cpct_seed3_xsp40_u8 == .+1
   xor #0xFD       ;; [2] A = w' ^ v' (as this operand contains updated v' value)
   ld  l, a        ;; [1] L = A       (put resulting value as return value)
  
   ret             ;; [3] Return final 8 pseudo-random bits