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
;; Function: cpct_getRandom_mxor_u16
;;
;;    Gets a high-quality 16-bit pseudo-random number using Marsaglia's XOR-shift 
;; algorithm (using a 32-bits state)
;;
;; C Definition:
;;    <u16> <cpct_getRandom_mxor_u16> ();
;;
;; Assembly call:
;;    > call cpct_getRandom_mxor_u16_asm
;;
;; Return value (Assembly calls, return HL=random 16-bits):
;;    <u16> - Next 16-bits pseudo-random value.
;;
;; Known limitations:
;;    * This function *cannot be used from ROM*, as is uses self-modifying code.
;;
;; Details:
;;    This function uses <cpct_nextRandom_mxor_u32> to produce a stream of random 
;; 32-bits numbers, and then gets 2 16-bits values out of each 32-bits number generated.
;; Then, it returns each one of the last 2 random values produced before calling 
;; <cpct_nextRandom_mxor_u32> again. It uses <cpct_mxor32_seed> as state storage buffer 
;; for the 32-bits number got from <cpct_nextRandom_mxor_u32>.
;;
;;    As <cpct_nextRandom_mxor_u32> produces (2^32)-1 32-bits numbers without repetition, 
;; this function will produce (2^33)-1 16-bits values without repetition.
;;
;;    It is important to know that this function will have 2 different behaviors:
;;    * [*available*] 1 out of every 2 calls, it will only return the next random 16-bits value
;; from the buffer (<cpct_mxor32_seed>).
;;    * [*production*] 1 out of every 2 calls, it will also call <cpct_nextRandom_mxor_u32> 
;; to produce a new 32-bits number (2 new 16-bits values).
;;    This behaviour is reflected in the time measures.
;;
;;    This function uses Marsaglia's XOR-shift standard algorithm with a concrete shift tuple. 
;; Check <cpct_nextRandom_mxor_u32> to know details on how this is produced.
;;
;; Destroyed Register values: 
;;      AF, BC, DE, HL
;;
;; Required memory:
;;    73 bytes divided in, 
;;    *  34 bytes (this function's code)
;;    * +35 bytes (from <cpct_nextRandom_mxor_u32> function's code)
;;    *  +4 bytes (from <cpct_mxor32_seed>)
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;  available  |      16        |     64
;;  production |      76        |    304
;; -----------------------------------------
;;  average-2  |      46        |    184
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Globals required for linking
.globl _cpct_mxor32_seed
.globl cpct_nextRandom_mxor_u32_asm

;; Include opcodes for self-modifying code
.include "macros/cpct_opcodeConstants.h.s"

_cpct_getRandom_mxor_u16::
cpct_getRandom_mxor_u16_asm::

cpct_seedJump_mxor_u16:
next_jump:
   jr   generate    ;; [3] Jumps to "generate" 1 out of 2 times, to generate the next 2 random words
;; ld d,#generate   ;; [2] This instruction is modified by next code, to become LD D,xx half of the time.
                    ;;     LD D, xx is a dummy instruction whose only purpose is not to jump

   ;; Return second word as random value
   ld   hl, (_cpct_mxor32_seed+0)  ;; [5] HL = second word (Most Significant one)
   ld    a, #opc_JR                ;; [2] | Set initial instruction to be a JR (0x18) for next call
   ld (next_jump), a               ;; [4] | This will jump to "generate", to generate 2 new random words
   ret                             ;; [3] Returns next 16-bits random value

generate:
   ;; Calculate next 32-bits random value using Marsaglia's XOR-shift
   ld   de, (_cpct_mxor32_seed+0)     ;; [6] | 
   ld   hl, (_cpct_mxor32_seed+2)     ;; [5] | DE:HL = Current 32-bits seed
   call cpct_nextRandom_mxor_u32_asm  ;; [5+37] Calculate next 32-bits pseudo-random value
   ld   (_cpct_mxor32_seed+0), de     ;; [6] |
   ld   (_cpct_mxor32_seed+2), hl     ;; [5] | Store new value as next seed

   ;; Return first word as random value (HL already holds it)
   ld    a, #opc_LD_D              ;; [2] | Set initial instruction to be LD E, xx (0x16) for next call
   ld (next_jump), a               ;; [4] | This is a dummy instruction set only to prevent jumping to "generate"
   ret                             ;; [3] Returns next 16-bits random value
