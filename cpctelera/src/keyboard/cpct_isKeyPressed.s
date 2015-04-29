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
;; Function: cpct_isKeyPressed
;;
;;    Checks if a concrete key is pressed or not. 
;;
;; C Definition:
;;    u8 *cpct_isKeyPressed* (<cpct_keyID> *key*);
;;
;; Input Parameters (2 Bytes):
;;  (2B C A) key - A 16-bit value containing a Matrix Line(1B, C) and a Bit Mask(1B, A).
;; 
;;
;; Parameter Restrictions:
;;  * *key* must be a valid <cpct_keyID>, containing a Matrix Line (1st byte, 0-9) and a 
;; Bit Mask (2nd byte, only 1 bit enabled). All keyID values are defined in 
;; <cpct_keyID> enum. Giving any other value is possible, but returned value
;; would be meaningless. If given value asks for a Matrix Line greater than 9,
;; unexpected results may happen.
;;
;; Return value:
;;    u8 - *false* (0, if not pressed) or *true* (>0, if pressed). Take into
;; account that *true* is not 1, but any non-0 number.
;;
;; Details:
;;    Checks if a concrete key is pressed or not. It does it looking   
;; at the <cpct_keyboardStatusBuffer>, which is an 80-bit array holding
;; the pressed / not pressed status of each of the 80 keys in the CPC
;; keyboard. Matrix Line is used to determine whick of the 10 bytes in 
;; the buffer contains the bit associated to the key, then Bit Mask is 
;; used to get the concrete bit using XOR and AND operations.
;;
;;    The <cpct_keyboardStatusBuffer> is just an array in memory that must
;; be updated with current key status. To do this, <cpct_scanKeyboard> 
;; routines must be used before calling this function.
;;
;; Destroyed Register values: 
;;    A, D, BC, HL
;;
;; Required memory:
;;    17 bytes
;;
;; Time Measures:
;; (start code)
;; Case | Cycles | microSecs (us)
;; -------------------------------
;; Any  |   95   |    23.25 
;; -------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl cpct_keyboardStatusBuffer

_cpct_isKeyPressed::
   ;; Get Parameters from stack
   ld   hl, #2                      ;; [10] HL = SP + 2 (Place where parameters start) 
   ld    b,  h                      ;; [ 4] B = 0 (We need B to be 0 later, and here we save 3 cycles against a ld B, #0)
   add  hl, sp                      ;; [11]
   ld    c, (hl)                    ;; [ 7] C = First Parameter (KeyID - Matrix Line)
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
