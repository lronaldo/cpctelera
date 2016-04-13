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
;; Function: cpct_setSeed_glfsr16
;;
;;    Sets machine state (seed) for 16-bits Galois Linear-Feedback Shift Register
;; pseudo-random number generator.
;;
;; C Definition:
;;    void <cpct_setSeed_glfsr16> (<u16> *newseed*) __z88dk_fastcall;
;;
;; Input Parameters (2 bytes):
;;    (2B HL) newseed - new machine state (seed) to be set as internal state for the
;; G-LFSR random number generator.
;;
;; Assembly call:
;;    > call cpct_setSeed_glfsr16_asm
;;
;; Parameter Restrictions:
;;    * *newseed* - Any value can be used as seed *except 0*. If you seed the G-LFSR
;; random number generator with 0, you will get a always 0 sequence.
;;
;; Known limitations:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;
;; Details:
;;    It sets the seed that 16-bits Galois Linear-Feedback Shift Register (G-LSFR16)
;; will use from now on to produce pseudo-random numbers. This is useful in two
;; main cases:
;;
;;    * You want to produce always the same pseudo-random sequence
;;    * You want to produce a different pseudo-random sequence
;;
;;    In the first case, initializing the pseudo-random number generator always with
;; the same *seed* will produce exactly the same sequence. Producing the same sequence
;; let you repeat a generation completely (for instance, if you are generating a labyrinth,
;; you may generate the same one again). On the contrary, the second case requires the
;; seed to be as random as possible. If you introduce a more-or-less random feed, the
;; sequence will be different each time. For this purpose, you may get pseudo-random
;; inputs like noise coming from cassette tape reader, or time between keypresses of
;; a user while typing input. 
;;
;;    With respect to the pseudo-random sequence generated, take into account that 
;; G-LSFR16 algorithms implement a 65535 numbers concrete pseudo-random sequence. 
;; When you set the seed, you are picking up which one of that 65535 numbers will
;; be the start of the sequence, but that sequence is cyclic. If you want the 
;; sequence to have a different internal order, you may use <cpct_setTaps_glfsr16>
;; along with one of the 1024 different possible sequences, codified in a TAPSET
;; from <GLFSR16_TAPS>.
;;
;; Destroyed Register values: 
;;      All preserved
;;
;; Required memory:
;;      4 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |       8        |     32
;; -----------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_randUnif_glfsr16_seed

_cpct_setSeed_glfsr16::
cpct_setSeed_glfsr16_asm::
   ;; HL holds parameter, as this function is __z88dk_fastcall
   ld  (cpct_randUnif_glfsr16_seed), hl ;; [5] Set new seed (Internal Galois Machine State)
   ret                                  ;; [3] Return
