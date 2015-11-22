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
;; Function: cpct_setSeed_mxor
;;
;;    Sets the new 32-bits seed value for Marsaglia's XOR-shift random number generator.
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
;; random number generator. It should never be set to 0. Functions that use this seed will always
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
;;    All functions calling <cpct_nextRandom_mxor_u32> or using <cpct_mxor32_seed> will be 
;; affected by this change.
;;
;; Destroyed Register values: 
;;      -
;;
;; Required memory:
;;      12 bytes divided in,
;;      *  8 bytes (this functions code)
;;      * +4 bytes (32-bits <cpct_mxor32_seed> seed value)
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any     |      14        |    52
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Globals required for linking
.globl _cpct_mxor32_seed

_cpct_setSeed_mxor::
cpct_setSeed_mxor_asm::
   ld   (_cpct_mxor32_seed+0), de     ;; [6] |
   ld   (_cpct_mxor32_seed+2), hl     ;; [5] | Store new seed in cpct_mxor32_seed
   ret                                ;; [3] return
