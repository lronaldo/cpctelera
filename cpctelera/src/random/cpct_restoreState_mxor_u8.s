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
;; Function: cpct_restoreState_mxor_u8
;;
;;    Restores internal index state of Marsaglia's XORShift 8-bits generator.
;;
;; C Definition:
;;    void <cpct_restoreState_mxor_u8> ();
;;
;; Assembly call:
;;    > call cpct_restoreState_mxor_u8_asm
;;
;; Known limitations:
;;    * This function *cannot be used from ROM*, as is uses self-modifying code.
;;
;; Details:
;;    Restores internal index counter of <cpct_getRandom_mxor_u8> to its initial
;; value (1). This ensures that next call to <cpct_getRandom_mxor_u8> will produce
;; 32 new random bits. Therefore, user will be sure that next value returned by
;; <cpct_getRandom_mxor_u8> is both pseudo-random generated and predictable (knowing
;; the seed that generates it).
;;
;;    Internal index counter of <cpct_getRandom_mxor_u8> is a counter that goes from
;; 4 to 0. Each time counter gets to 0, <cpct_nextRandom_mxor_u32> is called to produce
;; 32 new pseudo-random bits. If counter is not 0, next group of 8-bits from the 
;; last 32 pseudo-random bits is returned, using counter as index. Therefore, setting
;; this index counter to 1 ensures that next call to <cpct_getRandom_mxor_u8> will 
;; produce a call to <cpct_nextRandom_mxor_u32>, obtaining 32 new pseudo-random bits.
;; This is what this function does.
;;
;;    It is recommended to call this function after <cpct_setSeed_mxor> to get a
;; proper pseudo-random 8-bits sequence. 
;;
;; Destroyed Register values:
;;      A
;;
;; Required memory:
;;    6 bytes
;;    +37 bytes of <cpct_getRandom_mxor_u8>
;;
;; Time Measures:
;; (start code)
;;  Case  | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;  any   |       9        |     36
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; We need to change the value of the index counter to set it to its initial value
.globl cpct_randomIdx_mxor_u8

_cpct_restoreState_mxor_u8::
cpct_restoreState_mxor_u8_asm::
   ;; Set internal index counter to 1, which is the initial value. This is so
   ;; because next time cpct_getRandom_mxor_u8 gets called, 1-1 = 0 and 
   ;; cpct_nextRandom_mxor_u32 will get called, producing next random 32-bits
   ;; out of present seed value
   ld    a, #1                      ;; [2] A = 1
   ld (cpct_randomIdx_mxor_u8), a   ;; [4] Set cpct_randomIdx_mxor_u8 = 1, starting sequence value

   ret                              ;; [3] Returns next 8-bits random value
