;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 - 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;
.module cpct_keyboard
;
;########################################################################
;### FUNCTION: _cpct_isKeyPressed                                     ###
;########################################################################
;### Checks if a concrete key is pressed or not. It does it looking   ###
;### at the keyboardStatusBuffer, which is filled up by scan routines.###
;### So, take into account that keyboard has to be scanned before     ###
;### using this routine or it won't work.                             ###
;########################################################################
;### INPUTS (2B)                                                      ###
;###   -> KeyID, which contains Matrix Line(1B, C) and Bit Mask(1B, A)### 
;########################################################################
;### OUTPUTS (1B)                                                     ###
;###   -> True if the selected key is pressed, False otherwise.       ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: A, BC, HL                            ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  97/100 cyles (24.25/25.00 us)                                   ###
;########################################################################
;
;; Keyboard Status Buffer defined in an external file
.globl cpct_keyboardStatusBuffer

.globl _cpct_isKeyPressed
_cpct_isKeyPressed::
   ;; Get Parameters from stack
   LD  HL, #2                       ;; [10] HL = SP + 2 (Place where parameters start) 
   LD   B,  H                       ;; [ 4] B = 0 (We need B to be 0 later, and here we save 3 cycles against a LD B, #0)
   ADD HL, SP                       ;; [11]
   LD   C, (HL)                     ;; [ 7] C = First Parameter (KeyID - Matrix Line)
   INC HL                           ;; [ 6] 
   LD   A, (HL)                     ;; [ 7] A = Second Parameter (KeyID - Bit Mask)

   LD  HL,#cpct_keyboardStatusBuffer;; [10] Make HL Point to &keyboardStatusBuffer
   ADD HL, BC                       ;; [11] Make HL Point to &keyboardStatusBuffer + Matrix Line (C) (As B is already 0, so BC = C)
   AND (HL)                         ;; [ 7] A = AND operation between Key's Bit Mask (A) and the Matrix Line of the Key (HL)
   JP  NZ,  ikp_returnFalse         ;; [10] If AND resulted non-zero, Key's bit was 1, what means key was not pressed (return false = 0)
   LD   L, #0x01                    ;; [ 7] Else, Key's bit was 0, what means key was pressed (return true, L = 1)
   RET                              ;; [10] Return
ikp_returnFalse:
   LD   L,  B                       ;; [ 4] Return false (L = 0)
   RET                              ;; [10] Return
