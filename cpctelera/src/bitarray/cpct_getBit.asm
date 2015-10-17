;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;  Copyright (C) 2015 Alberto García García
;;  Copyright (C) 2015 Pablo Martínez González
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
.module cpct_bitarray

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_getBit
;;
;;    Returns the status of a given bit into a bitarray (0 or !0)
;;
;; C Definition:
;;    <u8> <cpct_getBit> (void* *array*, <u16> *index*);
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the bit in the array to be retrieved
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. So, be warned that giving mistaken values to this function will
;; not make it fail, but giving an unpredictable return result.
;;    * *index* position of the bit to be retrieved from the array, starting
;; in 0. As this function does not perform any boundary check, if you gave
;; an index outside the boundaries of the array, the return result would
;; be unpredictable and meaningless.
;;
;; Return value:
;;    u8 - Status of the selected bit: *false* (0) when the bit value is 0, 
;; *true* (> 0) when the bit value is 1. Take into account that >0 means any
;; value different than 0, and not necessarily 1.
;;
;; Known limitations:
;;    * Maximum of 65536 bits, 8192 bytes per *array*.      
;;
;; Details:
;;    Returns 0 or >0 depending on the value of the bit at the given 
;; position (*index*) in the specified *array*. It will assume that the 
;; array elements have a size of 8 bits and also that the given position 
;; is not bigger than the number of bits in the array (size of the array 
;; multiplied by 8).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;      C-bindings - 31 bytes
;;    ASM-bindings - 28 bytes
;;      bitWeights - +8 bytes vector required by both bindings. Take into 
;; account that this vector is included only once if you use different 
;; functions referencing to it.
;;
;; Time Measures:
;; (start code)
;;    Case    | microsec (ms) | Cycles
;; --------------------------------------
;;    Any     |      44       |   176
;; --------------------------------------
;; ASM-Saving |     -12       |   -48
;; --------------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.globl cpct_bitWeights

   ;; We only access bytes at once in memory. We need to calculate which
   ;; bit will we have to test in the target byte of our array. That will be
   ;; the remainder of INDEX/8, as INDEX/8 represents the byte to access.
   ld    bc, #cpct_bitWeights ;; [3] BC = Pointer to the start of the bitWeights array
   ld    a, l        ;; [1]
   and   #0x07       ;; [2] A = L % 8       (bit number to be tested from the target byte of the array) 
   add   c           ;; [1] A += C          (bit number is used as index, to increment BC, and 
                     ;; ....                  make BC point to its corresponding bit weight in the table)
   ld    c, a        ;; [1] BC = BC + L % 8 (Most of the time BC now points to the corresponding bitweight 
                     ;; ....                  in the table, except when A += C generates carry)
   sub   a           ;; [1] A  = 0          (preserving the carry, because we have to add it to B)
   adc   b           ;; [1] A  = B + Carry  (Add carry to B)
   ld    b, a        ;; [1] B += Carry      (Move result to B, to ensure BC now points to the bitweight)
   ld    a, (bc)     ;; [2] A  = BC [L % 8]  (bit weight to be tested in the target byte of the array)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 8 index positions (8 bits)
   ;; So, first, we calculate INDEX/8 (HL/8) to know the target byte.
   srl   h           ;; [2]
   rr    l           ;; [2]
   srl   h           ;; [2]
   rr    l           ;; [2]
   srl   h           ;; [2]
   rr    l           ;; [2] HL = HL / 8 (Target byte index into the array pointed by DE)

   ;; Reach the target byte and test the bit using the bit weight stored in A
   add  hl, de       ;; [3] HL += DE => HL points to the target byte in the array 
   and  (hl)         ;; [2] Test the selected bit in the target byte in the array
   ld   l, a         ;; [1] Return value (0 if bit is not set, !=0 if bit is set)

   ret               ;; [3] Return to caller