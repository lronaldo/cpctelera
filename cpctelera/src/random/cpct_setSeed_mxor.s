;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_setSeed_mxor
;;
;;    Sets the new 32-bits seed value for Marsaglia's XOR-shift random number 
;; generator.
;;
;; C Definition:
;;    void <cpct_setSeed_mxor> (<u32> *newseed*) __z88dk_fastcall;
;;
;; Input Parameters (4 bytes):
;;    (4B DE:HL) *newseed* - New seed to be set for the random number generator.
;;
;; Assembly call:
;;    > call cpct_setSeed_mxor_asm
;;
;; Parameter Restrictions:
;;    * *newseed* is the new seed byte that will be set internally for Marsaglia's XOR-shift 
;; random number generator. It should *never be set to 0*. Functions that use this seed will always
;; return 0 if this seed is set to 0.
;;
;; Details:
;;    This function sets the internal 32-bits seed used by Marsaglia's XOR-shift random number
;; generator. This generator is implemented by the function <cpct_nextRandom_mxor_u32> and its
;; internal seed is hold by the variable <cpct_mxor32_seed>. This last variable is the one
;; that this function changes. Under some concrete circumstances, directly setting 
;; <cpct_mxor32_seed> instead of calling this function could be a nice optimization. However,
;; take into account that this approach could not be portable among different versions of
;; this API.
;;
;;    If you are using <cpct_getRandom_mxor_u8> or <cpct_getRandom_mxor_u16>, only setting the
;; seed will not ensure you get the same sequence. Depending on the internal index state of
;; this two generators, you may get part of your seed as first random values. To prevent this
;; from happening, you have to call <cpct_restoreState_mxor_u8> or <cpct_restoreState_mxor_u16>
;; right after setting the seed with <cpct_setSeed_mxor>. This will ensure you get a predictable
;; random sequence.
;;
;;    Setting the mxor seed affects all functions using that seed, namely <cpct_nextRandom_mxor_u32>, 
;; <cpct_nextRandom_mxor_u16> and <cpct_nextRandom_mxor_u8>.
;;
;; Destroyed Register values: 
;;      All preserved
;;
;; Required memory:
;;      8 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any     |       14       |    56
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl _cpct_mxor32_seed

_cpct_setSeed_mxor::
cpct_setSeed_mxor_asm::
   ;; Only sets the seed. Does not restore internal index state of MXOR generators.
   ld   (_cpct_mxor32_seed+0), de     ;; [6] |
   ld   (_cpct_mxor32_seed+2), hl     ;; [5] | Store new value as next seed

   ret                                ;; [3]