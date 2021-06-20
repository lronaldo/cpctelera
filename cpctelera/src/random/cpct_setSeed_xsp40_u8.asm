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
;; Function: cpct_setSeed_xsp40_u8
;;
;;    Sets 40-bits initial state (seed) for <cpct_getRandom_xsp40_u8> pseudo-random 
;; number generator.
;;
;; C Definition:
;;    void <cpct_setSeed_xsp40_u8> (<u16> *plusSeed*, <u32> *seed32*) __z88dk_callee;
;;
;; Input Parameters (5 bytes):
;;    (4B DE:HL) *seed32*   - Main 32-bits seed for generating pseudo-random numbers
;;    (1B     A) *plusSeed* - Value that is used to do a final XOR operation with resulting
;; sequence numbers (Default: 255)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setSeed_xsp40_u8_asm
;;
;; Parameter Restrictions:
;;    * *seed32* can be any 32-bits value *except 0*. If 0 is set as seed, the random 
;; number generator's internal state will never be modifiyed. This would yield a 
;; poor quality pseudo-random pequence with period 256.
;;    * *plusSeed* can be any 8-bits number. This parameter is passed as 16-bits value
;; (in a C call), but only 8 Least Significan bits are used. It is recommended to use
;; odd numbers for *plusSeed*, as even numbers do not yield good quality results.
;;
;; Important details:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function sets the initial internal 40-bits state for the pseudo-random
;; number generator implemented in <cpct_getRandom_xsp40_u8>. It is important to 
;; properly seed the random number generator, never setting its 32-bits main seed
;; as 0. 
;;
;;    The internal state has 2 parts,
;;    * The 32-bits seed (*seed32*), which is the number from which next pseudo-random
;; states are produced. This is the proper seed of the algorithm.
;;    * The plus modifier (*plusSeed*), which is part of the way Marsaglia's XORShift+
;; algorithms operate. After moving the 32-bits state and calculating 8 new random 
;; bits (discarding the oldest 8), result is XORed with this *plusSeed* value, which
;; has the property of improving the quality of the pseudo-random sequence and enlarging it.
;;
;;    Therefore, you may have different sequences by selecting a different *plusSeed*,
;; whereas you will start the sequence in a different number by setting a new *seed32*.
;; As each sequence has a period of 2^40-256 numbers, any new sequence could be considered
;; approximately random once a relatively random seed has been set.
;;
;; Destroyed Register values: 
;;      AF, BC, DE, HL
;;
;; Required memory:
;;      C-bindings - 16 bytes
;;    ASM-bindings - 11 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |      34        |    136
;; -----------------------------------------
;;  ASM-Saving |     -16        |    -64
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl _cpct_seed1_xsp40_u8
.globl _cpct_seed2_xsp40_u8
.globl _cpct_seed3_xsp40_u8

   ld (_cpct_seed2_xsp40_u8), de ;; [6] Save first 16-bits seed from the 32-bits part (x, z values)
   ld (_cpct_seed1_xsp40_u8), hl ;; [5] Save second 16-bits seed from the 32-bits part (y, w values)
   ld (_cpct_seed3_xsp40_u8), a  ;; [4] Save 8-bits seed for the Weyl sequence (v value)

   ret         ;; [3] Return