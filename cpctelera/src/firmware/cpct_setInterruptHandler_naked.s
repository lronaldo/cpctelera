;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

.module cpct_firmware

.include /firmware.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setInterruptHandler_naked
;;
;;    Sets a user provided function as new interrupt handler.
;;
;; C Definition:
;;    void <cpct_setInterruptHandler_naked> ( void (*intHandler)(void) )
;;
;; Input Parameters (2 Bytes):
;;   (2B HL) intHandler - A pointer to the function that will handle interrupts from now on
;;
;; Assembly call:
;;    > call cpct_setInterruptHandler_naked_asm
;;
;; Parameter Restrictions:
;;  * *intHandler* (HL) can theoretically be any 16-bit value. However, it must point to
;; the start of a function that will be called at each interrupt. If it points to somewhere else,
;; undefined behaviour will happen. Most probably, the program will crash as random code will
;; be executed (that placed at the provided address). User defined function (*intHandler*)
;; must not return any value nor accept any parameters.
;;
;; Details:
;;    It modifies the interrupt vector to call user provided function. The user must
;; provide a pointer to the function (*intHandler*) that will handle interrupts.
;; This function does not call *intHandler* or check it in any way. It just sets it
;; for being called at the next interrupts. Provided *intHandler* function must not
;; return any value nor accept any parameter.
;;
;;    As opposed to <cpct_setInterruptHandler> or <cpct_setInterruptHandler_allRegisters>,
;; this function does not create wrapper code to safely call user provided *intHandler*.
;; This function assumes that provided *intHandler* saves and restores all needed registers.
;;
;; Destroyed Register values:
;;    A, HL
;;
;; Required memory:
;;    11 bytes
;;
;; Time Measures:
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; -------------------------------------
;; Any  |      16        |    64
;; -------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setInterruptHandler_naked::
cpct_setInterruptHandler_naked_asm::
   ;; Modify interrupt vector to call user provided interrupt handler
   ld  a, #0xC3                            ;; [2] 0xC3 = JP instruction, that may have been removed by other functions
   di                                      ;; [1] Disable interrupts
   ld (firmware_RST_jp), a                 ;; [4] Add JP at the start of interrupt vector's code
   ld (firmware_RST_jp+1), hl              ;; [5] Place interrupt handler pointer after JP in the interrupt vector's code
   ei                                      ;; [1] Reenable interrupts

   ret                                     ;; [3] Return
