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
; Function: cpct_isKeyPressed
;
;########################################################################
;### 
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
;###  Destroyed Register values: A, D, BC, HL                         ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  95 cyles (23.75 us)                                             ###
;########################################################################
; 
;; Keyboard Status Buffer defined in an external file
.globl cpct_keyboardStatusBuffer

_cpct_isKeyPressed::
   ;; Get Parameters from stack
   ld   hl, #2                      ;; [10] HL = SP + 2 (Place where parameters start) 
   ld    b,  h                      ;; [ 4] B = 0 (We need B to be 0 later, and here we save 3 cycles against a ld B, #0)
   add  hl, sp                      ;; [11]
   ld    C, (hl)                    ;; [ 7] C = First Parameter (KeyID - Matrix Line)
   inc  hl                          ;; [ 6] 
   ld    a, (hl)                    ;; [ 7] A = Second Parameter (KeyID - Bit Mask)
   ld    d, a                       ;; [ 4] D = A, save the Bit Mask into D for later use

   ld   hl,#cpct_keyboardStatusBuffer;; [10] Make HL Point to &keyboardStatusBuffer
   add  hl, bc                      ;; [11] Make HL Point to &keyboardStatusBuffer + Matrix Line (C) (As B is already 0, so BC = C)
   xor (hl)                         ;; [ 7] A = XOR operation between Key's Bit Mask (A) and the Matrix Line of the Key (HL)
                                    ;; .... Inverts the value of the bit associated to the given key that represents 
                                    ;; .... because 1 represents not pressed and 0 pressed, but we want the inverse
   and   d                          ;; [ 4] AND with the Bit Mask: leaves out only the bit associated to the key

   ld    l, a                       ;; [ 4] Place the return value in L (0=not pressed, >0=pressed)
   ret                              ;; [10] Return
