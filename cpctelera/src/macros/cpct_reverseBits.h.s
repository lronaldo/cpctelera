;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;
;; File: Reverse Bits
;;
;;    Useful macros for bit reversing and selecting in different ways. Only
;; valid to be used from assembly language (not from C).
;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_reverse_and_select_bits_of_A
;;
;;    Reorders the bits of A and mixes them letting the user select the 
;; new order for the bits by using a selection mask.
;;
;; Parameters:
;;    TReg          - An 8-bits register that will be used for intermediate calculations.
;; This register may be one of these: B, C, D, E, H, L
;;    SelectionMask - An 8-bits mask that will be used to select the bits to get from 
;; the reordered bits. It might be an 8-bit register or even (hl).
;; 
;; Input Registers: 
;;    A     - Byte to be reversed
;;    TReg  - Should have a copy of A (same exact value)
;;
;; Return Value:
;;    A - Resulting value with bits reversed and selected 
;;
;; Details:
;;    This macro reorders the bits in A and mixes them with the same bits in
;; their original order by using a *SelectionMask*. The process is as follows:
;;
;;    1. Consider the 8 bits of A = TReg = [01234567]
;;    2. Reorder the 8 bits of A, producing A2 = [32547610]
;;    2. Reorder the bits of TReg, producing TReg2 = [76103254]
;;    3. Combines both reorders into final result using a *SelectionMask*. Each 
;; 0 bit from the selection mask means "select bit from A2", whereas each 1 bit
;; means "select bit from TReg2".
;;
;;    For instance, a selection mask 0b11001100 will produce this result:
;;
;; (start code)
;;       A2 = [ 32 54 76 10 ]
;;    TReg2 = [ 76 10 32 54 ]
;;  SelMask = [ 11 00 11 00 ] // 1 = TReg2-bits, 0 = A2-bits
;;  ---------------------------
;;   Result = [ 76 54 32 10 ]
;; (end code)
;;
;;    Therefore, mask 0b11001100 produces the effect of reversing the bits of A
;; completely. Other masks will produce different reorders of the bits in A, for
;; different requirements or needs.
;;
;; Modified Registers: 
;;    AF, TReg
;;
;; Required memory:
;;    16 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |      16       |     64
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro cpctm_reverse_and_select_bits_of_A  TReg, SelectionMask
   rlca            ;; [1] | Rotate left twice so that...
   rlca            ;; [1] | ... A=[23456701]

   ;; Mix bits of TReg and A so that all bits are in correct relative order
   ;; but displaced from their final desired location
   xor TReg        ;; [1] TReg = [01234567] (original value)
   and #0b01010101 ;; [2]    A = [23456701] (bits rotated twice left)
   xor TReg        ;; [1]   A2 = [03254761] (TReg mixed with A to get bits in order)
   
   ;; Now get bits 54 and 10 in their right location and save them into TReg
   rlca            ;; [1]    A = [ 32 54 76 10 ] (54 and 10 are in their desired place)
   ld TReg, a      ;; [1] TReg = A (Save this bit location into TReg)
   
   ;; Now get bits 76 and 32 in their right location in A
   rrca            ;; [1] | Rotate A right 4 times to...
   rrca            ;; [1] | ... get bits 76 and 32 located at their ...
   rrca            ;; [1] | ... desired location :
   rrca            ;; [1] | ... A = [ 76 10 32 54 ] (76 and 32 are in their desired place)
   
   ;; Finally, mix bits from TReg and A to get all bits reversed and selected
   xor TReg          ;; [1] TReg = [32547610] (Mixed bits with 54 & 10 in their right place)
   and SelectionMask ;; [2]    A = [76103254] (Mixed bits with 76 & 32 in their right place)
   xor TReg          ;; [1]   A2 = [xxxxxxxx] final value: bits of A reversed and selected using *SelectionMask*
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_reverse_bits_of_A 
;; Macro: cpctm_reverse_mode_2_pixels_of_A
;;
;;    Reverses the 8-bits of A, from [01234567] to [76543210]. This also reverses
;; all pixels contained in A when A is in screen pixel format, mode 2.
;;
;; Parameters:
;;    TReg - An 8-bits register that will be used for intermediate calculations.
;; This register may be one of these: B, C, D, E, H, L
;; 
;; Input Registers: 
;;    A    - Byte to be reversed
;;    TReg - Should have a copy of A (same exact value)
;;
;; Return Value:
;;    A - Resulting value with bits reversed 
;;
;; Requires:
;;   - Uses the macro <cpctm_reverse_and_select_bits_of_A>.
;;
;; Details:
;;    This macro reverses the bits in A. If bits of A = [01234567], the final
;; result after processing this macro will be A = [76543210]. Register TReg is
;; used for intermediate calculations and its value is destroyed.
;;
;; Modified Registers: 
;;    AF, TReg
;;
;; Required memory:
;;    16 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |      16       |     64
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro cpctm_reverse_bits_of_A  TReg
   cpctm_reverse_and_select_bits_of_A  TReg, #0b11001100
.endm
.macro cpctm_reverse_mode_2_pixels_of_A   TReg
   cpctm_reverse_bits_of_A  TReg
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_reverse_mode_1_pixels_of_A
;;
;;    Reverses the order of pixel values contained in register A, assuming A is 
;; in screen pixel format, mode 1.
;;
;; Parameters:
;;    TReg - An 8-bits register that will be used for intermediate calculations.
;; This register may be one of these: B, C, D, E, H, L
;; 
;; Input Registers: 
;;    A    - Byte with pixel values to be reversed
;;    TReg - Should have a copy of A (same exact value)
;;
;; Return Value:
;;    A - Resulting byte with the 4 pixels values reversed in order
;;
;; Requires:
;;   - Uses the macro <cpctm_reverse_and_select_bits_of_A>.
;;
;; Details:
;;    This macro considers that A contains a byte that codifies 4 pixels in 
;; screen pixel format, mode 1. It modifies A to reverse the order of its 4 
;; contained pixel values left-to-right (1234 -> 4321). With respect to the 
;; order of the 8-bits of A, the concrete operations performed is:
;; (start code)
;;    A = [01234567] == reverse-pixels ==> [32107654] = A2
;; (end code)
;;    You may want to check <cpct_px2byteM1> to know how bits codify both pixels
;; in one single byte for screen pixel format, mode 1.
;;
;;    *TReg* is an 8-bit register that will be used for intermediate calculations,
;; destroying its original value (that should be same as A, at the start).
;;
;; Modified Registers: 
;;    AF, TReg
;;
;; Required memory:
;;    16 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |      16       |     64
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro cpctm_reverse_mode_1_pixels_of_A  TReg
   cpctm_reverse_and_select_bits_of_A  TReg, #0b00110011
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_reverse_mode_0_pixels_of_A
;;
;;    Reverses the order of pixel values contained in register A, assuming A is 
;; in screen pixel format, mode 0.
;;
;; Parameters:
;;    TReg - An 8-bits register that will be used for intermediate calculations.
;; This register may be one of these: B, C, D, E, H, L
;; 
;; Input Registers: 
;;    A    - Byte with pixel values to be reversed
;;    TReg - Should have a copy of A (same exact value)
;;
;; Return Value:
;;    A - Resulting byte with the 2 pixels values reversed in order
;;
;; Details:
;;    This macro considers that A contains a byte that codifies 2 pixels in 
;; screen pixel format, mode 0. It modifies A to reverse the order of its 2 
;; contained pixel values left-to-right (12 -> 21). With respect to the 
;; order of the 8-bits of A, the concrete operation performed is:
;; (start code)
;;    A = [01234567] == reverse-pixels ==> [10325476] = A2
;; (end code)
;;    You may want to check <cpct_px2byteM0> to know how bits codify both pixels
;; in one single byte for screen pixel format, mode 0.
;;
;;    *TReg* is an 8-bit register that will be used for intermediate calculations,
;; destroying its original value (that should be same as A, at the start).
;;
;; Modified Registers: 
;;    AF, TReg
;;
;; Required memory:
;;    7 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |       7       |     28
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro cpctm_reverse_mode_0_pixels_of_A  TReg
   rlca            ;; [1] | Rotate A twice to the left to get bits ordered...
   rlca            ;; [1] | ... in the way we need for mixing, A = [23456701]
  
   ;; Mix TReg with A to get pixels reversed by reordering bits
   xor TReg        ;; [1] | TReg = [01234567]
   and #0b01010101 ;; [2] |    A = [23456701]
   xor TReg        ;; [1] |   A2 = [03254761]
   rrca            ;; [1] Rotate right to get pixels reversed A = [10325476]
.endm