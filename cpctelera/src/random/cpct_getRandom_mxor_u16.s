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
;; Function: cpct_getRandom_mxor_u16
;;
;;    Gets a high-quality 16-bit pseudo-random number using Marsaglia's XOR-shift 
;; algorithm.
;;
;; C Definition:
;;    <u16> <cpct_getRandom_mxor_u16> ();
;;
;; Assembly call:
;;    > call cpct_getRandom_mxor_u16_asm
;;
;; Known limitations:
;;    * This function *cannot be used from ROM*, as is uses self-modifying code.
;;
;; Details:
;;    This function uses <cpct_nextRandom_mxor_u32> to produce a stream of random 
;; words (16-bits each word) in groups of 2. Then, it returns each one of the last 2 random 
;; words produced previous to calling <cpct_nextRandom_mxor_u32> again. It uses 
;; <cpct_mxor32_seed> as storage buffer for the last 2 random words got from 
;; <cpct_nextRandom_mxor_u32>.
;;
;;    It is important to know that this function will have 2 different behaviors:
;;    * [*available*] 1 out of every 2 calls, it will only return the next random-word 
;; from the buffer (<cpct_mxor32_seed>).
;;    * [*production*] 1 out of every 2 calls, it will also call <cpct_nextRandom_mxor_u32> 
;; to produce 2 new random words.
;;    This behaviour is reflected in the time measures.
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

;; Opcodes used as data
JR_opcode   = 0x18
LD_E_opcode = 0x16

;; Globals required for linking
.globl _cpct_mxor32_seed
.globl cpct_nextRandom_mxor_u32_asm

_cpct_getRandom_mxor_u16::
cpct_getRandom_mxor_u16::

next_jump:
   jr   generate    ;; [3] Jumps to "generate" 1 out of 2 times, to generate the next 2 random words
;; ld e,#generate   ;; [2] This instruction is modified by next code, to become LD E,xx half of the time.
                    ;;     LD E, xx is a dummy instruction whose only purpose is not to jump

   ;; Return second word as random value
   ld   hl, (_cpct_mxor32_seed+0)  ;; [5] HL = second word (Most Significant one)
   ld    a, #JR_opcode             ;; [2] | Set initial instruction to be a JR (0x18) for next call
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
   ld    a, #LD_E_opcode           ;; [2] | Set initial instruction to be LD E, xx (0x16) for next call
   ld (next_jump), a               ;; [4] | This is a dummy instruction set only to prevent jumping to "generate"
   ret                             ;; [3] Returns next 16-bits random value
