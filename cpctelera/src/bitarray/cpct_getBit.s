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
;; multiplied by eight).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    42 bytes (8 bytes bitWeights table, 33 bytes code)
;;
;; Time Measures:
;; (start code)
;; Case | Cycles | microSecs (us)
;; -------------------------------
;; Any  |   191  |  47.75
;; -------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; .bndry 8 ;; Make this vector start at a 8-byte aligned address to be able to use 8-bit arithmetic with pointers
;; bndry does not work when generated object file is linked later on.
cpct_bitWeights:: .db #0x01, #0x02, #0x04, #0x08, #0x10, #0x20, #0x40, #0x80

_cpct_getBit::
   ;; Get parameters from the stack
   pop   af          ;; [10] AF = Return address
   pop   de          ;; [10] DE = Pointer to the array in memory
   pop   hl          ;; [10] HL = Index of the bit we want to get
   push  hl          ;; [11] << Restore stack status
   push  de          ;; [11]
   push  af          ;; [11]

   ;; We only access bytes at once in memory. We need to calculate which
   ;; bit will we have to test in the target byte of our array. That will be
   ;; the remainder of INDEX/8, as INDEX/8 represents the byte to access.
   ld    bc, #cpct_bitWeights ;; [10] BC = Pointer to the start of the bitWeights array
   ld    a, l        ;; [ 4]
   and   #0x07       ;; [ 7] A = L % 8       (bit number to be tested from the target byte of the array) 
   add   c           ;; [ 4] A += C          (bit number is used as index, to increment BC, and make BC point to its corresponding bit weight in the table)
   ld    c, a        ;; [ 4] BC = BC + L % 8 (Most of the time BC now points to the corresponding bitweight in the table, except when A += C generates carry)
   sub   a           ;; [ 4] A  = 0          (preserving the carry, because we have to add it to B)
   adc   b           ;; [ 4] A  = B + Carry  (Add carry to B)
   ld    b, a        ;; [ 4] B += Carry      (Move result to B, to ensure BC now points to the bitweight)
   ld    a, (bc)     ;; [ 7] A  = BC [L % 8]  (bit weight to be tested in the target byte of the array)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 8 index positions (8 bits)
   ;; So, first, we calculate INDEX/8 (HL/8) to know the target byte.
   srl   h           ;; [ 8]
   rr    l           ;; [ 8]
   srl   h           ;; [ 8]
   rr    l           ;; [ 8]
   srl   h           ;; [ 8]
   rr    l           ;; [ 8] HL = HL / 8 (Target byte index into the array pointed by DE)

   ;; Reach the target byte and test the bit using the bit weight stored in A
   add  hl, de       ;; [11] HL += DE => HL points to the target byte in the array 
   and  (hl)         ;; [ 7] Test the selected bit in the target byte in the array
   ld   l, a         ;; [ 4] Return value (0 if bit is not set, !=0 if bit is set)

   ret               ;; [10] Return to caller
