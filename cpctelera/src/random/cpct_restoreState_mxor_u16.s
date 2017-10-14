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
;; Function: cpct_restoreState_mxor_u16
;;
;;    Restores internal index state of Marsaglia's XORShift 16-bits generator.
;;
;; C Definition:
;;    void <cpct_restoreState_mxor_u16> ();
;;
;; Assembly call:
;;    > call cpct_restoreState_mxor_u16_asm
;;
;; Known limitations:
;;    * This function *cannot be used from ROM*, as is uses self-modifying code.
;;
;; Details:
;;    Restores internal of <cpct_getRandom_mxor_u16> to its initial value (jump generate). 
;; This ensures that next call to <cpct_getRandom_mxor_u16> will produce 32 new random bits. 
;; Therefore, user will be sure that next value returned by <cpct_getRandom_mxor_u16> is 
;; both pseudo-random generated and predictable (knowing the seed that generates it).
;;
;;    Internal state of <cpct_getRandom_mxor_u16> has 2 possible states: "jump generate" and
;; "no-jump". First state produces function to jump to its generating code section when called
;; to generate 32 new random bits, returning 16 of those. Next state of the function simply 
;; returns the other 16 random bits (hence the no-jump, for not jumping to the generate section).
;; So, restoring the initial state is adding a "JR" instruction at the first byte of the 
;; function. That's what this function does.
;;
;;    It is recommended to call this function after <cpct_setSeed_mxor> to get a
;; proper pseudo-random 16-bits sequence. 
;;
;; Destroyed Register values:
;;      A
;;
;; Required memory:
;;    6 bytes
;;    +34 bytes of <cpct_getRandom_mxor_u16>
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
.globl cpct_seedJump_mxor_u16
.include "macros/cpct_opcodeConstants.h.s"

_cpct_restoreState_mxor_u16::
cpct_restoreState_mxor_u16_asm::
   ;; Set cpct_getRandom_mxor_u16 internal state to "jump generate", which ensures
   ;; that next call to the function will also call cpct_nextRandom_mxor_u32, producing
   ;; 32 new random bits.
   ld    a, #opc_JR                 ;; [2] A = Opcode for JR, xx instruction
   ld (cpct_seedJump_mxor_u16), a   ;; [4] Set seedJump_mxor_u16 to "jump generate" placing a JR, xx

   ret                              ;; [3] Return
