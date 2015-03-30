;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_memset

.include /sprites.s/

;
;########################################################################
;### FUNCTION: cpct_memset                                            ###
;########################################################################
;### Sets all the bytes of an array in memory to the same given value ###
;### This is the technique it uses:                                   ###
;###  - It first sets up the first byte of the array to the value     ###
;###  - Makes HL point to the first byte and DE to the second         ###
;###  - BC has the total bytes to copy minus 1 (the first already set)###
;###  - LDIR copies first byte into second, then second into third... ###
;### WARNING: This works only for values of BC > 1. Never use this    ###
;### function with values 0 or 1 as number of bytes to copy, as it    ###
;### will have unexpected results (In fact, it could write up the en- ###
;### tire memory).                                                    ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (2B DE) Starting point in memory                              ###
;###  * (2B BC) Number of bytes to be set (should be greater than 1!) ###
;###  * (1B A) Value to be set                                        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES (Way 2 for parameter retrieval from stack)              ###
;### MEMORY:  17 bytes                                                ###
;### TIME:    120 + 21*BC cycles (28.25 + 5.25*BC us)                 ###
;########################################################################
;

.globl _cpct_memset
_cpct_memset::

.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   LD  HL, #0        ;; [10] HL = SP (To quickly restore it later)
   ADD HL, SP        ;; [11]
   DI                ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP AF            ;; [10] Get return value from stack
   POP DE            ;; [10] (1st param) DE = Starting point in memory
   POP BC            ;; [10] (2nd param) BC = Number of bytes to be set
   DEC SP            ;; [ 6] ** 3rd param is byte-size, so need to decrease SP to load it into A when doing POP (otherwise it would go to F)
   POP AF            ;; [10] (3rd param) A = Value to be set
ms_restoreSP:
   LD SP, HL         ;; [ 6] -- Restore Stack Pointer --
   EI                ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af           ;; [10] Get return value from stack
   pop  de           ;; [10] (1st param) DE = Starting point in memory
   pop  bc           ;; [10] (2nd param) BC = Number of bytes to be set
   pop  hl           ;; [10] (3rd param) L = Value to be set (H=random)
   push hl           ;; [11] Restore Stack status pushing values again
   push bc           ;; [11] (Interrupt safe way, 7 cycles more)
   push de           ;; [11]
   push af           ;; [11]
   ld   a, l         ;; [ 4] A = L (Value to be set)
.endif

   ;; Set up HL and DE for a massive copy of the Value to be set
   LD H, D           ;; [ 4] HL = DE (Starting address in memory)
   LD L, E           ;; [ 4] 
   INC DE            ;; [ 6] DE++ (Next position in memory)
   LD (HL), A        ;; [ 7] (HL) = A --> First item of the memory array = Value
   DEC BC            ;; [ 6] BC--, We already have copied the first byte with previous instruction

   LDIR              ;; [21/16] Copy the reset of the bytes, cloning previous one

   RET               ;; [10] Return 
