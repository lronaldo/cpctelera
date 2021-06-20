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
;; Function: cpct_isAnyKeyPressed_f
;;
;;    Checks if there is at least one key pressed. This function does the same as
;; <cpct_isAnyKeyPressed> but ~73% faster.
;;
;; C Definition:
;;    <u8> <cpct_isAnyKeyPressed_f> ();
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_isAnyKeyPressed_f_asm
;;
;; Return value:
;;    <u8> - *false* (0, no single key is pressed) or *true* (>0, at least one key
;; is pressed). Take into account that *true* is not 1, but any non-0 number. Return 
;; value is placed in registers A and L (same value for both)
;;
;; Details:
;;    Checks if at least one key from the keyboard is pressed. It does it looking   
;; at the <cpct_keyboardStatusBuffer>, which is an 80-bit array holding the pressed / not pressed 
;; status of each of the 80 keys in the CPC keyboard. If at least one key is pressed, 
;; one of the 80-bits representing the keys must be set to 0 (which means that key
;; is pressed right now). This is exactly the same as its brother function <cpct_isAnyKeyPressed>
;; but using an unrolled version of the loop that makes it ~73% faster. However, it's counterpart
;; is requiring more space for performing the operation. 
;;
;;    The <cpct_keyboardStatusBuffer> is just an array in memory that *must
;; be updated* with current key status. To do this, <cpct_scanKeyboard> 
;; routines *must be* used before calling this function.
;;
;; Destroyed Register values: 
;;    A, B, HL
;;
;; Required memory:
;;       27 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      | microSecs (us) | CPU Cycles 
;; -------------------------------------------
;; Any         |       48       |    192
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl _cpct_keyboardStatusBuffer

_cpct_isAnyKeyPressed_f::
cpct_isAnyKeyPressed_f_asm::
   ld  hl, #_cpct_keyboardStatusBuffer ;; [3] HL Points to the start of the keyboard status buffer
   ld   b, #9           ;; [2] We are going to do 9 AND operations against the first byte of the buffer
   ld   a, (hl)         ;; [2] A = First byte from keyboardStatusBuffer

   inc  hl              ;; [2] HL points to the next byte from the KeyboardStatusBuffer
   and (hl)             ;; [2] A = A & NextByte (The byte pointed by HL)
   inc  hl              ;; [2] Repeat for byte Buffer+2
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+3
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+4
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+5
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+6
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+7
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+8
   and (hl)             ;; [2] 
   inc  hl              ;; [2] Repeat for byte Buffer+9
   and (hl)             ;; [2] 

   inc   a              ;; [1] A holds the result of ANDing the 10 bytes. If no key is pressed, all bits should
                        ;; ... be 1, so A=0xFF. If we add 1, A=0, we return FALSE (no key is pressed).
                        ;; ... If any key is pressed, some bit will be 0, so A != 0xFF, which means A+1 != 0, and 
                        ;; ... we will be returning TRUE (A > 0)
   ld    l, a           ;; [1] L = A (Set return value for C calls in L)
   ret                  ;; [3] Return