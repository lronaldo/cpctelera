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
;; Function: cpct_getRandom_mxor_u32
;;
;;    Gets a 32-bit pseudo-random number using Marsaglia's XOR-shift algorithm
;; (using a 32-bits state)
;;
;; C Definition:
;;    <u32> <cpct_getRandom_mxor_u32> ();
;;
;; Assembly call:
;;    > call cpct_getRandom_mxor_u32_asm
;;
;; Return value (Assembly calls, return DE:HL=random 16-bits):
;;    <u32> - Next 32-bits pseudo-random value.
;;
;; Known limitations:
;;    * This function will never produce a 0 as random 32-bits integer value. If you 
;; require a 0 to be produced, use returned value minus 1.
;;    * This function does not repeat a single 32-bits value in the sequence it produces
;; until it finishes its period ((2^32)-1). Use other functions if you require random
;; numbers to sometimes repeat values (which is often desirable)
;;
;; Details:
;;    This function uses <cpct_nextRandom_mxor_u32> to produce a sequence of 
;; 32-bits pseudo-random numbers. It uses <cpct_mxor32_seed> to store the last returned 
;; random 32-bits value: this is required to produce the next value in the sequence.
;;
;;    To know more about how these 32-bits numbers are produced, check 
;; <cpct_nextRandom_mxor_u32> documentation.
;;
;; Destroyed Register values: 
;;      AF, BC, DE, HL
;;
;; Required memory:
;;    57 bytes divided in, 
;;    *  18 bytes (this function's code)
;;    * +35 bytes (from <cpct_nextRandom_mxor_u32> function's code)
;;    *  +4 bytes (from <cpct_mxor32_seed>)
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any     |      67        |    268
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Globals required for linking
.globl _cpct_mxor32_seed
.globl cpct_nextRandom_mxor_u32_asm

_cpct_getRandom_mxor_u32::
cpct_getRandom_mxor_u32::
   ;; Calculate next 32-bits random value using Marsaglia's XOR-shift
   ld   de, (_cpct_mxor32_seed+0)     ;; [6] | 
   ld   hl, (_cpct_mxor32_seed+2)     ;; [5] | DE:HL = Current 32-bits seed
   call cpct_nextRandom_mxor_u32_asm  ;; [5+37] Calculate next 32-bits pseudo-random value
   ld   (_cpct_mxor32_seed+0), de     ;; [6] |
   ld   (_cpct_mxor32_seed+2), hl     ;; [5] | Store new value as next seed

   ret                                ;; [3] Returns next 32-bits random value (DE:HL)
