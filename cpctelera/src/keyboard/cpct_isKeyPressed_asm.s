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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_isKeyPressed_asm
;;
;;    Assembly callable version of <cpct_isKeyPressed> (only callable from ASM)
;;
;; Input Parameters (2 Bytes):
;;  (2B BC) keyId - A 16-bit value containing a Matrix Line(1B, C) and a Bit Mask(1B, B).
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_isKeyPressed_asm
;;
;; Parameter Restrictions:
;;  * *keyID* (BC) must be a valid <cpct_keyID>, containing a Matrix Line (1st 
;; byte, 0-9) and a Bit Mask (2nd byte, only 1 bit enabled). All keyID values 
;; are defined in <cpct_keyID> enum. Giving any other value is possible, but 
;; returned value would be meaningless. If given value asks for a Matrix Line 
;; greater than 9, unexpected results may happen.
;;
;; Return value:
;;    A - *false* (0, if not pressed) or *true* (>0, if pressed). Take into
;; account that *true* is not 1, but any non-0 number.
;;
;; Details:
;;    Performs the same operation as <cpct_isKeyPressed> but is designed to 
;; be called from assembly code only. It receives a <cpct_keyID> value on
;; BC register and checks if that key was pressed at the last keyboard scan.
;; A register holds return value, with a 0 if key was not pressed, and a 
;; value different than 0 (not necessary 1) if the key was pressed.
;;
;; Destroyed Register values: 
;;    A, BC, D, HL
;;
;; Required memory:
;;    11 bytes
;;
;; Time Measures:
;; (start code)
;; Case | Cycles | microSecs (us)
;; -------------------------------
;; Any  |   47   |    11.75
;; -------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl _cpct_keyboardStatusBuffer

cpct_isKeyPressed_asm::
   ld    a, b     ;; [ 4] A = (KeyID - Bit Mask)
   ld    d, a     ;; [ 4] D = A (Save A for later use)
   ld    b, #0    ;; [ 7] B = 0 (To make BC = C and then HL += BC equivalent to HL += C)

   ld   hl,#_cpct_keyboardStatusBuffer;; [10] Make HL Point to &keyboardStatusBuffer
   add  hl, bc    ;; [11] Make HL Point to &keyboardStatusBuffer + Matrix Line (C) (As B = 0, so BC = C)
   xor (hl)       ;; [ 7] A = XOR operation between Key's Bit Mask (A) and the Matrix Line of the Key (HL)
                  ;; .... Inverts the value of the bit associated to the given key that represents 
                  ;; .... because 1 represents not pressed and 0 pressed, but we want the inverse
   and   d        ;; [ 4] AND with the Bit Mask: leaves out only the bit associated to the key

   ret            ;; [10] Return
