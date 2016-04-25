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
;; Function: cpct_getRandom_lcg_u8
;;
;;    Returns a pseudo-random byte using a fast Linear-Congruential-Algebra (LCG)
;; method (33*Seed mod 257)
;;
;; C Definition:
;;    <u8> <cpct_getRandom_lcg_u8> (<u8> *entropy_byte*) __z88dk_fastcall;
;;
;; Input Parameters (1 byte):
;;    (1B L) entropy_byte - An optional byte coming from an entropy source to mix up the sequence. 
;;                          Use 0 if you have no entropy byte.
;;
;; Assembly call (Input parameter on L):
;;    > call cpct_getRandom_lcg_u8_asm
;;
;; Return value (Assembly calls, return L=A=random 8-bits):
;;    <u8> - Next 8-bits pseudo-random value.
;;
;; Parameter Restrictions:
;;    * *entropy_byte* this parameter is a byte to be used as sequence mixer. This byte is
;; XORed with previous seed before calculating next pseudo-random number from the sequence.
;; There is no restriction for this parameter. If you use 0 as *entropy_byte*, the pseudo-random
;; generator will act normally, returning the general (33*Seed mod 257) sequence. You may 
;; give it values coming from entropy sources like number of iterations of a loop until a 
;; user presses a key, or location of an object in the screen that depends on user interaction,
;; etc. That may improve the quality of the random sequence.
;;
;; Known limitations:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;
;; Important details:
;;    * This function uses a previous 8-bits seed to calculate next pseudo-random byte, and 
;; stores returned byte minus 1 as seed for the next time. If you want a reproducible sequence
;; you may set the seed using <cpct_setSeed_lcg_u8> to ensure that all calls
;; to this function (using 0 as parameter) always return the same sequence. You also 
;; may use this as a way to randomize the sequence, by starting with a seed that comes out
;; of an entropy source.
;;
;; Details:
;;    This function returns a pseudo-random byte using a Linear-Congruential-Algebra. Each new 
;; byte returned is obtained as a result of the next function,
;; (start code)
;;    Seed = (33*Seed % 257) - 1;
;;    return Seed + 1;
;; (end code)
;;    Seed is then updated to the value of the LCG function minus 1, to be used for the 
;; next call to the function. The seed may be manually set using the function <cpct_setSeed_lcg_u8> 
;; to create predictable sequences (starting always with the same seed) or to randomize the start 
;; of a the sequence (by entering a seed coming out of an entropy source).
;;
;;    Linear-Congruential-Algebra methods are known for being really simple and low-quality
;; methods. As Marsaglia noted in his works, LCG methods produce numbers equally distributed 
;; in predictable hyperplanes in the space, which makes them highly predictable and low-quality.
;; However, these methods might come in handy due to their simplicity, as their cost is also
;; really low.
;;
;;    The parameter *entropy_byte* is an addition to this function to let the user improve
;; the random quality of the sequence by inputting mixer bytes coming from entropy sources.
;; The function does an XOR operation between *entropy_byte* and the previous seed before
;; doing the calculations for the next random byte, this way,
;; (start code)
;;    Seed = (33*(Seed ^ entropy_byte) % 257) - 1;
;;    return Seed + 1;
;; (end code)
;;    Therefore, if you use *entropy_byte*s coming from good entropy sources, you will be 
;; effectively mixing up the sequence and getting better quality random numbers. This could 
;; also be used for other purposes, like creating restricted or repeating sequences, up to 
;; the user imagination.
;;
;;    If you have no entropy source or do not want to use the *entropy_byte*, just input 0
;; which will have no effect on the sequence.
;;
;; Destroyed Register values: 
;;      AF, C, L
;;
;; Required memory:
;;      17 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |      20        |    80
;; -----------------------------------------
;; (end code)
;;
;; Credits:
;;    This function comes from Fast RND, which can be found <published by z80-info at
;; http://www.z80.info/pseudo-random.txt>. The original code is reported to be from
;; Spectrum ROM.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_getRandom_lcg_u8::
cpct_getRandom_lcg_u8_asm::
   ;; No need to recover parameters from stack. This function uses,
   ;;  __z88dk_fastcall convention so its unique 8-bits parameter is in L.
cpct_randlcgu8_seed == .+1
   ld   a, #00      ;; [2] A = Seed
   ld   c, a        ;; [1] C = Copy of A
   
   xor  l           ;; [1] Use parameter in L = Entropy source (mesh up bits)

   rrca             ;; [1] | A = A * 32 
   rrca             ;; [1] |   (without taking Carry into account)
   rrca             ;; [1] |
   xor  #0x1F       ;; [2] |

   add  c           ;; [1] A  = 32A + A = 33A
   sbc  #0xFF       ;; [2] A += 1 - Carry

   ld (cpct_randlcgu8_seed), a ;; [4] Save new random number as seed
   ld   l, a        ;; [1] L = Return parameter
   ret              ;; [3] Return
