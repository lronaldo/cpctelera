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
;; Function: cpct_set2Bits
;;
;;    Sets the value of a selected group of 2 bits into a bitarray to [0-3]
;;
;; C Definition:
;;    void <cpct_set2Bits>  (void* *array*, <u16> *value*, <u16> *index*)
;;
;; Input Parameters (6 Bytes, B register ignored, only C register is used for value):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the group of 2 bits in the array to be modified
;;    (2B BC) value - New value {0, 1, 2, 3} for the group of 2 bits at the given position. If 
;; you call from assembly, you can safely ignore B register and set only C register.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_set2Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. Giving and incorrect *array* pointer will have unpredictable
;; results: a random group of 2 bits from your memory may result changed.
;;    * *value* new value for the selected group of 2 bits [0-3]. Only the 
;; 2 Least Significant Bits (LSBs) are used. This means that any given *value* 
;; will "work": *value* module 4 will be finally inserted in the *array*
;; position (*index*).
;;    * *index* position of the group of 2 bits to be modified in the array, 
;; starting in 0. Again, as this function does not perform any boundary check, 
;; if you gave an index outside the boundaries of the array, a group of 2 bits 
;; outside the array will be changed in memory, what will have unpredictable results.
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
;;      C-bindings - 57 bytes 
;;    ASM-bindings - 53 bytes 
;;
;; Time Measures:
;; (start code)
;; Case       | microSecs(us) | CPU Cycles
;; -----------------------------------------
;; Best  (0)  |       55      |    220
;; -----------------------------------------
;; 2nd   (3)  |       57      |    228
;; -----------------------------------------
;; 3rd   (1)  |       59      |    236
;; -----------------------------------------
;; Worst (2)  |       61      |    244
;; -----------------------------------------
;; Asm saving |      -15      |    -60
;; -----------------------------------------
;; 
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be set
   ;;   inside the target byte ([ 00 11 22 33 ]).
   ld    a, l        ;; [1] A = L (Least significant bits from array index)
   and   #0x03       ;; [2] A = L % 4 (Calculate the group of 2 bytes we will be setting from the target byte:
                     ;; ....             the remainder of L/4)
   ld    b, a        ;; [1] B = index of the group of 2 bits that we want to set from 0 to 3 ([00 11 22 33])
   ld    a, c        ;; [1] A = C     (Value to be set)
   and   #0x03       ;; [2] A = C % 4 (Ensure value to be set is in the range 0-3)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 4 index positions (8 bits)
   ;; So, first, we calculate INDEX/4 (HL/4) to know the target byte.
   srl   h           ;; [2]
   rr    l           ;; [2]
   srl   h           ;; [2]
   rr    l           ;; [2] HL = HL / 4 (HL holds byte offset to advance into the array pointed by DE)
   add  hl, de       ;; [3] HL += DE => HL points to the target byte in the array 

   ;; Move the 2 new bits to be set from bits 0 & 1 to their final position in the target byte
   ;;  and setting up a mask to be used for inserting them in the target byte of the array
   inc   b                ;; [1]   We do B++; B-- so to set Z flag if B was 0.
   dec   b                ;; [1]   This lets us use JR jumps which are better than JP
   jr    z, s2b_B_is_0    ;; [2/3] IF B=0, jump to B is 0
   dec   b                ;; [1]   B--
   jr   nz, s2b_B_is_2or3 ;; [2/3] If B!=0, jump to B is 2 or 3, if B=0, B was 1, so continue
s2b_B_is_1:
   rrca                ;; [1] <| Move bits to positions 5 & 4 using 4 right rotations
   rrca                ;; [1] <|
   rrca                ;; [1] <|
   rrca                ;; [1] <|
   ld    c, #0xCF      ;; [2] Mask for leaving out bits 5 & 4
   jr s2b_end          ;; [3] Done with preparing mask and rotations, continue to setting bits
s2b_B_is_2or3:
   dec   b             ;; [1]   B-- (second time, so B-=2)
   jr   nz, s2b_B_is_3 ;; [2/3] IF B != 0, B was >2 (3), else B was 2
s2b_B_is_2:
   rlca                ;; [1] <| Move bits to positions  2 & 3 with 2 left rotations
   rlca                ;; [1] <|
   ld    c, #0xF3      ;; [2] Mask for leaving out bits 2 & 3
   jr s2b_end          ;; [3] Done with preparing mask and rotations, continue to setting bits
s2b_B_is_0:
   ld    c, #0x3F      ;; [2] Mask for leaving out bits 7 & 6
   rrca                ;; [1] <| Move bits to positions 7 & 6 with 2 right rotations
   rrca                ;; [1] <|
   .db #0xFA ;jp M, xx ;; [3] Fake jump to gb_end (JP M, xx will be never done, as S is not set. Value XX is got from 
                       ;; .... next 2 bytes, which are "ld C, #0xFC". Not jumping leaves us 3 bytes from here, at g2b_end)
s2b_B_is_3:
   ld    c, #0xFC      ;; [2] Mask for leaving out bits 1 & 0
s2b_end:
   ld    b, a          ;; [1] B = A (Bits to be set)
   ld    a, (hl)       ;; [2] A = target Byte
   and   c             ;; [1] A = target Byte Masked (Set to 0 the 2 bits we are going to set with our new value)
   or    b             ;; [1] A = final value (Set the new value for our new bits, ORing them with the other 3 2bit groups)
   ld  (hl), a         ;; [2] Store the final value of the target byte in the array

   ret                 ;; [3] Return to caller
