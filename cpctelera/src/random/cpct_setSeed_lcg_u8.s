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
;; Function: cpct_setSeed_lcg_u8
;;
;;    Sets the random seed used by <cpct_getRandom_lcg_u8>.
;;
;; C Definition:
;;    void <cpct_setSeed_lcg_u8> (<u8> *seed*) __z88dk_fastcall;
;;
;; Input Parameters (1 byte):
;;    (1B L) seed - New seed to be set for the random number generator.
;;
;; Assembly call (Input parameter on L):
;;    > call cpct_setSeed_lcg_u8_asm
;;
;; Parameter Restrictions:
;;    * *seed* New seed byte that will be set internally in the random number generator.
;; There is no restriction about the value of this byte.
;;
;; Known limitations:
;;    * This function *will not work from a ROM*. It does not specifically use self-modifying
;; code, but indirectly modifies the code of <cpct_getRandom_lcg_u8>.
;;
;; Details:
;;    This function sets the random *seed* that <cpct_getRandom_lcg_u8> uses internally
;; for generating pseudo-random numbers. This can be used to randomize the start of the 
;; pseudo-random sequence or to make it predictable (setting the same seed again).
;;
;;    Please, do read documentation from <cpct_getRandom_lcg_u8> for more details about
;; how this random generator works.
;;
;; Destroyed Register values: 
;;      A
;;
;; Required memory:
;;      2 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |       7        |    28
;; -----------------------------------------
;; (end code)
;;
;; Credits:
;;    Random generation original code comes from Fast RND, which can be found 
;; <published by z80-info at http://www.z80.info/pseudo-random.txt>. The original 
;; code is reported to be from Spectrum ROM.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_randlcgu8_seed

_cpct_setSeed_lcg_u8::
cpct_setSeed_lcg_u8_asm::
   ;; No need to recover parameters from stack. This function uses,
   ;;  __z88dk_fastcall convention so its unique 8-bits parameter is in L.

   ld   a, l                   ;; [1] A = new seed (Passed as parameter in L)
   ld (cpct_randlcgu8_seed), a ;; [4] Save parameter as new random seed

   ret                         ;; [3] Return
