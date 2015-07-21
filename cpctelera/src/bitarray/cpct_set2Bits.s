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

.include /bitarray.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_set2Bits
;;
;;    Sets the value of a selected group of 2 bits into a bitarray to [0-3]
;;
;; C Definition:
;;    void <cpct_set2Bits>  (void* *array*, <u16> *index*, <u8> *value*)
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the group of 2 bits in the array to be modified
;;    (1B C ) value - New value {0, 1, 2, 3} for the group of 2 bits at the given position
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_set2Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. Giving and incorrect *array* pointer will have unpredictable
;; results: a random group of 2 bits from your memory may result changed.
;;    * *index* position of the group of 2 bits to be modified in the array, 
;; starting in 0. Again, as this function does not perform any boundary check, 
;; if you gave an index outside the boundaries of the array, a group of 2 bits 
;; outside the array will be changed in memory, what will have unpredictable results.
;;    * *value* new value for the selected group of 2 bits [0-3]. Only the 
;; 2 Least Significant Bits (LSBs) are used. This means that any given *value* 
;; will "work": *value* module 4 will be finally inserted in the *array*
;; position (*index*).
;;
;; Known limitations:
;;    * Maximum of 65536 bits, 16384 bytes per array.
;;
;; Details:
;;    Set the new *value* of the 2-bits group at the given position (*index*) 
;; in the specified *array*. This function assumes that the *array* elements have 
;; a size of 8 bits and also that the given *index* is not bigger than 
;; the number of 2-bits groups in the *array* (size of the *array* multiplied
;; by 4). The *value* to be set is also assumed to be in the range [0-3] but 
;; other values will "work" (just the 2 Least Significant Bits will be used, 
;; so *value* module 4 will be inserted).
;; 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    64 bytes 
;;
;; Time Measures:
;; (start code)
;; Case       | Cycles | microSecs (us)
;; -------------------------------
;; Best  (0)  |   228  |  57.00
;; -------------------------------
;; Worst (1)  |   260  |  65.00
;; -------------------------------
;; Asm saving |   -84  | -21.00
;; -------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_set2Bits::
   ;; GET Parameters from the stack
   pop  af                  ;; [10] AF = Return Address
   pop  de                  ;; [10] DE = Pointer to the bitarray in memory
   pop  hl                  ;; [10] HL = Index of the bit to be set
   pop  bc                  ;; [10] BC => C = Set Value (0/3), B = Undefined
   push bc                  ;; [11] Restore Stack status pushing values again
   push hl                  ;; [11] (Interrupt safe way, 6 cycles more)
   push de                  ;; [11]
   push af                  ;; [11]

cpct_set2Bits_asm::         ;; Entry point for assembly calls using registers for parameter passing

   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be set
   ;;   inside the target byte ([ 00 11 22 33 ]).
   ld    a, l               ;; [ 4] A = L (Least significant bits from array index)
   and   #0x03              ;; [ 7] A = L % 4 (Calculate the group of 2 bytes we will be setting from the target byte:
                            ;; ....             the remainder of L/4)
   ld    b, a               ;; [ 4] B = index of the group of 2 bits that we want to set from 0 to 3 ([00 11 22 33])
   ld    a, c               ;; [ 4] A = C     (Value to be set)
   and   #0x03              ;; [ 7] A = C % 4 (Ensure value to be set is in the range 0-3)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 4 index positions (8 bits)
   ;; So, first, we calculate INDEX/4 (HL/4) to know the target byte.
   srl   h           ;; [ 8]
   rr    l           ;; [ 8]
   srl   h           ;; [ 8]
   rr    l           ;; [ 8] HL = HL / 4 (HL holds byte offset to advance into the array pointed by DE)
   add  hl, de       ;; [11] HL += DE => HL points to the target byte in the array 

   ;; Move the 2 new bits to be set from bits 0 & 1 to their final position in the target byte
   ;;  and setting up a mask to be used for inserting them in the target byte of the array
   dec   b             ;; [ 4] If B=0, B-- is negative, turning on S flag
   jp    m, s2b_B_is_0 ;; [10] IF S flag is on, B was 0
   dec   b             ;; [ 4] B-- (second time, so B-=2)
   jp    p, s2b_B_is_2or3 ;; [10] If S flag is off, B was > 1, else B was 1
s2b_B_is_1:
   rrca                ;; [ 4] <| Move bits to positions 5 & 4 using 4 right rotations
   rrca                ;; [ 4] <|
   ld    c, #0xCF      ;; [ 7] Mask for leaving out bits 5 & 4
   jp s2b_B_is_0 + 2   ;; [10] We have done 2 rotations, and jump to B_is_0 + 2, to make another 2 rotations
s2b_B_is_2or3:
   dec   b             ;; [ 4] B-- (third time, so B-=3)
   jp    p, s2b_B_is_3 ;; [10] IF S flag is off, B was >2 (3), else B was 2
s2b_B_is_2:
   rlca                ;; [ 4] <| Move bits to positions  2 & 3 with 2 left rotations
   rlca                ;; [ 4] <|
   ld    c, #0xF3      ;; [ 7] Mask for leaving out bits 2 & 3
   jp s2b_end          ;; [10] Done with preparing mask and rotations, continue to setting bits
s2b_B_is_0:
   ld    c, #0x3F      ;; [ 7] Mask for leaving out bits 7 & 6
   rrca                ;; [ 4] <| Move bits to positions 7 & 6 with 2 right rotations
   rrca                ;; [ 4] <|
   .db #0xF2 ;jp P, xx ;; [10] Fake jump to gb_end (JP P, xx will be never done, as S is set. Value XX is got from 
                       ;; .... next 2 bytes, which are "ld C, #0xFC". Not jumping leaves us 3 bytes from here, at g2b_end)
s2b_B_is_3:
   ld    c, #0xFC      ;; [ 7] Mask for leaving out bits 1 & 0
s2b_end:
   ld    b, a          ;; [ 4] B = A (Bits to be set)
   ld    a, (hl)       ;; [ 7] A = target Byte
   and   c             ;; [ 4] A = target Byte Masked (Set to 0 the 2 bits we are going to set with our new value)
   or    b             ;; [ 4] A = final value (Set the new value for our new bits, ORing them with the other 3 2bit groups)
   ld  (HL), A         ;; [ 7] Store the final value of the target byte in the array

   ret                 ;; [10] Return to caller
