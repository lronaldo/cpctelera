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
;; Function: cpct_get2Bits
;;
;;    Returns the value of a given group of 2 bits into an array (0, 1, 2 or 3)
;;
;; C Definition:
;;    <u8> <cpct_get2Bits> (void* *array*, <u16> *index*);
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the group of 2 bits to be retrieved from the array
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. So, be warned that giving mistaken values to this function will
;; not make it fail, but giving an unpredictable return result.
;;    * *index* position of the group of 2 bits to be retrieved from the array, 
;; starting in 0. As this function does not perform any boundary check, if you gave
;; an index outside the boundaries of the array, the return result would
;; be unpredictable and meaningless.
;;
;; Return value:
;;    u8 - Value of the selected group of 2 bits: 0, 1, 2 or 3
;;
;; Known limitations:
;;    * Maximum of 65536 groups of 2-bits, 16384 bytes per *array*.      
;;
;; Details:
;;    Returns 0, 1, 2 or 3 depending on the value of the 2-bits group at 
;; the given position (*index*) in the specified *array*. It will assume that 
;; the array elements have a size of 8 bits and also that the given position 
;; is not bigger than the number of bits in the array (size of the array 
;; multiplied by 4).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    46 bytes 
;;
;; Time Measures:
;; (start code)
;; Case      | Cycles | microSecs (us)
;; -----------------------------------
;; Best (0)  |   171  |  42.75
;; -----------------------------------
;; Worst (2) |   209  |  52.25
;; -----------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_get2Bits::

   ;; Get parameters from the stack
   pop   af          ;; [10] AF = Return address
   pop   de          ;; [10] DE = Pointer to the array in memory
   pop   hl          ;; [10] HL = Index of the bit we want to get
   push  hl          ;; [11] << Restore stack status
   push  de          ;; [11]
   push  af          ;; [11]

   ld    a, l        ;; [ 4] A = L 
   AND   #0x03       ;; [ 7] A = L % 4 (Calculate the group of 2 bytes we will be getting from the target byte: 
                     ;; ....            the remainder of L/4)
   ld    b, a        ;; [ 4] B = index of the group of 2 bits that we want to get from 0 to 3 ([00 11 22 33])

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 4 index positions (8 bits)
   ;; So, first, we calculate INDEX/4 (HL/4) to know the target byte.
   srl   h           ;; [ 8]
   rr    l           ;; [ 8]
   srl   h           ;; [ 8]
   rr    l           ;; [ 8] HL = HL / 4 (HL holds byte offset to advance into the array pointed by DE)
   add  hl, de       ;; [11] HL += DE => HL points to the target byte in the array 
   ld    a, (hl)     ;; [ 7] A = array[index] Get the byte where our 2 target bits are located

   ;; Move the 2 required bits to the least significant position (bits 0 & 1)
   ;;   This is done to make easier the opperation of returning a value from 0 to 3 (represented by the 2 bits searched).
   ;;   Once the bits are at least significant position, we only have to AND them and we are done.
   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be returned
   ;;   inside the target byte [ 0 0 | 1 1 | 2 2 | 3 3 ].
   dec   b             ;; [ 4] If B=0, B-- is negative, turning on S flag
   jp    m, g2b_B_is_0 ;; [10] IF S flag is on, B was 0
   dec   b             ;; [ 4] B-- (second time, so B-=2)
   jp    p, g2b_B_is_2or3 ;; [10] If S flag is off, B was > 1, else B was 1
g2b_B_is_1:
   rlca                ;; [ 4] <| Move bits 5 & 4 to positions 0 & 1 with 4 left rotations
   rlca                ;; [ 4] <|
   jp g2b_B_is_0       ;; [10] We have done 2 rotations, and jump to B_is_0, to make another 2 rotations
g2b_B_is_2or3:
   dec   b             ;; [ 4] B-- (third time, so B-=3)
   jp    p, g2b_end    ;; [10] IF S flag is off, B was >2 (3), else B was 2
g2b_B_is_2:
   rrca                ;; [ 4] <| Move bits 2 & 3 to positions 0 & 1 with 2 right rotations
   rrca                ;; [ 4] <|
   .db #0xF2 ;JP P, xx ;; [10] Fake jump to gb_end (JP P, xx will be never done, as S is set. Value XX is got 
                       ;; .... from next 2 bytes, which are RLCA;RLCA. Not jumping leaves us 3 bytes from here, at g2b_end)
g2b_B_is_0:
   rlca                ;; [ 4] <| Move bits 7 & 6 to positions 0 & 1 with 2 left rotations
   rlca                ;; [ 4] <|
g2b_end:                            ; 0:22, 1:54, 2:60 3:42
   and   #0x03         ;; [ 7] Leave out the 2 required bits (bits 0 & 1, at least significant positions).
   ld    l, a          ;; [ 4] Set the return value in L 

   ret                 ;; [10] Return to caller
