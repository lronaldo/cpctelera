;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;  Copyright (C) 2015 Alberto García García
;;  Copyright (C) 2015 Pablo Martínez González
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
;; Assembly call (Input parameters on registers):
;;    > call cpct_get2Bits_asm
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
;;    AF, DE, HL
;;
;; Required memory: 
;;    C-binding   - 38 bytes
;;    ASM-binding - 35 bytes
;;
;; Time Measures:  
;; (start code)
;; Case       | Microsecs | CPU Cycles
;; -----------------------------------
;; Best (3)   |    41     |   164
;; -----------------------------------
;; Worst (1)  |    47     |   188
;; -----------------------------------
;; ASM-saving |   -12     |   -48
;; -----------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Start of the function code (without bindings for calling)
;;

   ;; Store L in B to use it later for calculating the group of 2-bits
   ;; that will be access from the target byte. The 2-bits group is
   ;; determined by the 2 least significant bits (L % 4)
   ld    b, l        ;; [1] B = L

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 4 index positions (8 bits)
   ;; So, first, we calculate INDEX/4 (HL/4) to know the target byte.
   srl   h           ;; [2]
   rr    l           ;; [2]
   srl   h           ;; [2]
   rr    l           ;; [2] HL = HL / 4 (HL holds byte offset to advance into the array pointed by DE)
   add  hl, de       ;; [3] HL += DE => HL points to the target byte in the array 
   ld    a, (hl)     ;; [2] A = array[index] Get the byte where our 2 target bits are located

   ;; Move the 2 required bits to the least significant position (bits 0 & 1)
   ;;   This is done to make easier the opperation of returning a value from 0 to 3 (represented by the 2 bits searched).
   ;;   Once the bits are at least significant position, we only have to AND them and we are done.
   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be returned
   ;;   inside the target byte [ 0 0 | 1 1 | 2 2 | 3 3 ].  
   rr  b                 ; [2]   Check the bit 0 from L by moving it to the carry
   jr  nc, g2b_group_2_0 ; [2/3] If C=0 the 2 last bits of B containt 10 or 00
   rr  b                 ; [2]   C=1, the 2 last bits contain 11 or 01, move bit 1 to the carry to check
   jr  c, g2b_end        ; [2/3] IF C=1 selected bits are 1&0 (group 3), else selected bits are 5&4 (group 1)

   ;; 4 options, for the 4 possible groups of 2 bits
g2b_bits_54:
   rlca           ;; [1] Group 1: Bits 54 moved to 10 using 4 RLCA's
   rlca           ;; [1]
g2b_bits_76:
   rlca           ;; [1] Group 0: Bits 76 moved to 10 using 2 RLCA's
   rlca           ;; [1]
   jr g2b_end     ;; [3] Group 3: Nothing to do, as we are returning bits 10,
                  ;; ... so jump to the end
g2b_group_2_0:
   rr  b               ;; [2]   C=0, the 2 last bits contain 10 or 00, move bit 1 to the carry to check
   jr  nc, g2b_bits_76 ;; [2/3] IF C=0 selected bits are 7&6 (group 0), else selected bits are 3&2 (group 2)

g2b_bits_32:
   rrca           ;; [1] Option 2: Bits 32 moved to 10 using 2 RRCA's
   rrca           ;; [1]
g2b_end:           
   and   #0x03    ;; [2] Leave out the 2 required bits (bits 0 & 1, at least significant positions).
   ld    l, a     ;; [1] Set the return value in L 

   ret            ;; [3] Return to caller

