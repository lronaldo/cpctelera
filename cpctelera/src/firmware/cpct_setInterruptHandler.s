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

.module cpct_firmware

.include /firmware.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setInterruptHandler
;;
;;    Sets a user provided function as new interrupt handler.
;;
;; C Definition:
;;    void <cpct_setInterruptHandler> ( void (*intHandler)(void) )
;;
;; Input Parameters (2 Bytes):
;;   (2B HL) intHandler - A pointer to the function that will handle interrupts from now on
;; 
;; Assembly call:
;;    > call cpct_setInterruptHandler
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
;; code saves registers on the stack (AF, BC, DE, HL and IX) and restores them after 
;; user code from *intHandler* finishes. Therefore, user does not have to worry about
;; saving registers. However, *be very careful if you modify the alternate set of 
;; registers* (AF', BC', DE', HL') as they will not be saved by default. User is 
;; responsible for saving and restoring the values on the alternate register set
;; whenever they are modified. Otherwise, behaviour is undefined when returning from 
;; the function.
;;
;; Destroyed Register values: 
;;    A, HL
;;
;; Required memory:
;;    40 bytes (17 bytes function + 23 bytes for safe interrupt wrapper code)
;;
;; Time Measures:
;;  * This first measure is the time required for cpct_setInterruptHandler to 
;; execute: that is, the time for setting up the hook for the system to call
;; user defined code.
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; -------------------------------------
;; Any  |      23        |    92 
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
;; Overhead  |      58        |    184
;; ------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setInterruptHandler::
cpct_setInterruptHandler_asm::
   ld (cpct_safeInterruptHandlerCall+1), hl;; [5] Place the pointer to the user code just after the call instruction
   
   ;; Modify interrupt vector to call save interrupt handler hook (code below)
   ld  a, #0xC3                            ;; [2] 0xC3 = JP instruction, that may have been removed by other functions
   di                                      ;; [1] Disable interrupts
   ld (firmware_RST_jp), a                 ;; [4] Add JP at the start of interrupt vector's code
   ld hl, #cpct_safeInterruptHandlerHook   ;; [3] HL = pointer to safe interrupt handler hook
   ld (firmware_RST_jp+1), hl              ;; [5] Place interrupt handler hook pointer after JP in the interrupt vector's code
   ei                                      ;; [1] Reenable interrupts

   ret                                     ;; [3] Return

;;
;; Interrupt Handler Safe Wrapper Code. This is the code that
;; will be called at the start of the interrupt, and this code 
;; will call user defined function, after saving registers. It 
;; also returns using reti for user comfortability.
;;  Overhead: 48 microsecs
;;
cpct_safeInterruptHandlerHook::
   di          ;; [1] Disable interrupts
   push af     ;; [4] Save all standard registers on the stack
   push bc     ;; [4]
   push de     ;; [4]
   push hl     ;; [4]
   push ix     ;; [5]
   push iy     ;; [5]

cpct_safeInterruptHandlerCall:   
   call #0000  ;; [5] Call Interrupt Handler

   pop  iy     ;; [5] Restore all standard registers
   pop  ix     ;; [5]
   pop  hl     ;; [3]
   pop  de     ;; [3]
   pop  bc     ;; [3]
   pop  af     ;; [3]
   ei          ;; [1] Reenable interrupts
   reti        ;; [3] Return to main program