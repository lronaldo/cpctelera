;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 - 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;
.module cpct_keyboard

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_isKeyPressed
;;
;;    Checks if a concrete key is pressed or not. 
;;
;; C Definition:
;;    <u8> <cpct_isKeyPressed> (<cpct_keyID> *key*);
;;
;; Input Parameters (2 Bytes):
;;   (2B HL) key - A 16-bit value containing a Matrix-Line(1B, L) and a BitMask(1B, H).
;; 
;; Assembly call (Input parameters on registers):
;;    > call cpct_isKeyPressed_asm
;;
;; Parameter Restrictions:
;;  * *key* (HL) must be a valid <cpct_keyID>, containing a Matrix Line (1st byte, 
;; 0-9) and a Bit Mask (2nd byte, only 1 bit enabled). All keyID values are defined 
;; in <cpct_keyID> enum. Giving any other value is possible, but returned value
;; would be meaningless. If given value asks for a Matrix Line greater than 9,
;; unexpected results may happen.
;;
;; Return value (for Assembly, L=A=key_status):
;;    <u8> - *false* (0, if not pressed) or *true* (>0, if pressed). Take into
;; account that *true* is not 1, but any non-0 number. 
;;
;; Flag Output Status:
;;    - *Z* = 1 (Key NOT pressed) / 0 (Key pressed)
;;    - *C*,*N* = 0
;;    - *H* = 1
;;
;; Details:
;;    Checks if a concrete key is pressed or not. It does it looking   
;; at the <cpct_keyboardStatusBuffer>, which is an 80-bit array holding
;; the pressed / not pressed status of each of the 80 keys in the CPC
;; keyboard. Matrix Line is used to determine which of the 10 bytes in 
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
;;       12 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      | microSecs (us) | CPU Cycles 
;; -------------------------------------------
;; Any         |      17        |    68 
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl _cpct_keyboardStatusBuffer

;; Using __z88dk_fastcall calling convention. Parameter is passed directly in HL
_cpct_isKeyPressed::
cpct_isKeyPressed_asm::
   ld    a, h     ;; [1] A = BitMask with only 1 bit ON: the one that identifies the key (<TargetKey>)
   ld    d, a     ;; [1] D = A (Save BitMask for later use)
   ld    h, #0    ;; [2] H = 0 (HL = L, so that we can add the matrix line as offset from the start of the buffer)
   ld   bc, #_cpct_keyboardStatusBuffer ;; [3] BC Points to the start of the KeyboardStatusBuffer
   add  hl, bc    ;; [3] HL += BC (HL Points to the byte that contains the key
                  ;; ... we are looking for. Let's call it <TargetByte> )
   xor (hl)       ;; [2] A = <TargetByte> but with the bit of the key we want inverted (because 
                  ;; ... bytes store 1's for non-pressed keys and 0's for pressed, and we want
                  ;; ... the inverse if this)
   and   d        ;; [1] A = Only the bit representing the <TargetKey> is left after doing and 
                  ;; ... AND operation with the BitMask. The bit will have a 1 if <TargetKey> 
                  ;; ... was pressed, and a 0 otherwise
   ld    l, a     ;; [1] L = Return value (0 = <TargetKey> not pressed, 1 = pressed)
   ret            ;; [3] Return
