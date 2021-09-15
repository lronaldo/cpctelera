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
;; Function: cpct_setInterruptHandler_allRegisters
;;
;;    Sets a user provided function as new interrupt handler.
;;
;; C Definition:
;;    void <cpct_setInterruptHandler_allRegisters> ( void (*intHandler)(void) )
;;
;; Input Parameters (2 Bytes):
;;   (2B HL) intHandler - A pointer to the function that will handle interrupts from now on
;;
;; Assembly call:
;;    > call cpct_setInterruptHandler_allRegisters_asm
;;
;; Parameter Restrictions:
;;  * *intHandler* (HL) can theoretically be any 16-bit value. However, it must point to
;; the start of a function that will be called at each interrupt. If it points to somewhere else,
;; undefined behaviour will happen. Most probably, the program will crash as random code will
;; be executed (that placed at the provided address). User defined function (*intHandler*)
;; must not return any value nor accept any parameters.
;;
;; Details:
;;    It modifies the interrupt vector to establish a new interrupt handler that calls
;; user provided function. The user must provide a pointer to the function (*intHandler*)
;; that will handle interrupts. This function does not call *intHandler* or check it
;; in any way. It just sets it for being called at the next interrupts. Provided *intHandler*
;; function must not return any value nor accept any parameter.
;;
;;    This function creates wrapper code to safely call user provided *intHandler*. This
;; code saves all standard and alternate registers on the stack, and restores them after
;; user code from *intHandler* finishes. Therefore, user does not have to worry about
;; saving registers.
;;
;; Destroyed Register values:
;;    A, HL
;;
;; Required memory:
;;    51 bytes (17 bytes function + 34 bytes for safe interrupt wrapper code)
;;
;; Time Measures:
;;  * This first measure is the time required for cpct_setInterruptHandler_allRegisters to
;; execute: that is, the time for setting up the hook for the system to call
;; user defined code.
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; -------------------------------------
;; Any  |      24        |    96
;; -------------------------------------
;; (end code)
;;  * This second measure is the time overhead required for safely calling
;; user defined function. That is, time required by the wrapper code to save
;; registers, call user's *intHandler*, restoring the registers and returning.
;; This overhead is to be assumed each time interrupt handler is called, so
;; up to 6 times per frame, 300 times per second.
;; (start code)
;; Case      | microSecs (us) | CPU Cycles
;; ------------------------------------------
;; Overhead  |      88        |    352
;; ------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setInterruptHandler_allRegisters::
cpct_setInterruptHandler_allRegisters_asm::
   ld (cpct_safeInterruptHandlerCallAllRegs+1), hl;; [5] Place the pointer to the user code just after the call instruction

   ;; Modify interrupt vector to call save interrupt handler hook (code below)
   ld  a, #0xC3                                  ;; [2] 0xC3 = JP instruction, that may have been removed by other functions
   di                                            ;; [1] Disable interrupts
   ld (firmware_RST_jp), a                       ;; [4] Add JP at the start of interrupt vector's code
   ld hl, #cpct_safeInterruptHandlerHookAllRegs  ;; [3] HL = pointer to safe interrupt handler hook
   ld (firmware_RST_jp+1), hl                    ;; [5] Place interrupt handler hook pointer after JP in the interrupt vector's code
   ei                                            ;; [1] Reenable interrupts

   ret                                           ;; [3] Return

;;
;; Interrupt Handler Safe Wrapper Code. This is the code that
;; will be called at the start of the interrupt, and this code
;; will call user defined function, after saving registers. It
;; also returns using reti for user comfortability.
;;  Overhead: 89 microsecs
;;
cpct_safeInterruptHandlerHookAllRegs::
   push af     ;; [4] Save all standard registers on the stack
   push bc     ;; [4]
   push de     ;; [4]
   push hl     ;; [4]
   push ix     ;; [5]
   push iy     ;; [5]
   ex af,af'   ;; [1] Swap af/af'
   exx         ;; [1] Swap bc/bc' de/de' and hl/hl'
   push af     ;; [4] Save all alternate registers on the stack
   push bc     ;; [4]
   push de     ;; [4]
   push hl     ;; [4]
cpct_safeInterruptHandlerCallAllRegs:
   call #0000  ;; [5] Call Interrupt Handler
   pop  hl     ;; [3] Restore all alternate registers
   pop  de     ;; [3]
   pop  bc     ;; [3]
   pop  af     ;; [3]
   exx         ;; [1] Swap bc/bc' de/de' and hl/hl'
   ex af,af'   ;; [1] Swap af/af'
   pop  iy     ;; [4] Restore all standard registers
   pop  ix     ;; [4]
   pop  hl     ;; [3]
   pop  de     ;; [3]
   pop  bc     ;; [3]
   pop  af     ;; [3]
   ei          ;; [1] Reenable interrupts
   reti        ;; [4] Return to main program
