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
;; Function: cpct_get4Bits
;;
;;    Returns the value of a given group of 4 bits into an array ( [0-15] )
;;
;; C Definition:
;;    <u8> <cpct_get4Bits> (void* *array*, <u16> *index*);
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the group of 4 bits to be retrieved from the array
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_get4Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. So, be warned that giving mistaken values to this function will
;; not make it fail, but giving an unpredictable return result.
;;    * *index* position of the group of 4 bits to be retrieved from the array, 
;; starting in 0. As this function does not perform any boundary check, if you gave
;; an index outside the boundaries of the array, the return result would
;; be unpredictable and meaningless.
;;
;; Return value:
;;    u8 - Value of the selected group of 4 bits: [0-15]
;;
;; Known limitations:
;;    * Maximum of 65536 groups of 4-bits, 32768 bytes per *array*.      
;;
;; Details:
;;    Returns a value from 0 to 15 depending on the value of the 4-bits group at 
;; the given position (*index*) in the specified *array*. It will assume that 
;; the array elements have a size of 8 bits and also that the given position 
;; is not bigger than the number of bits in the array (size of the array 
;; multiplied by 2).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings   - 25 bytes 
;;    ASM-bindings - 22 bytes
;;
;; Time Measures:
;; (start code)
;; Case      | microSecs (us) | CPU Cycles |
;; -----------------------------------------
;; Best (1)  |      30        |     120    |
;; -----------------------------------------
;; Worst (0) |      33        |     132    |
;; -----------------------------------------
;; ASM Saving|     -12        |     -48    |
;; -----------------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;; We need to know how many bytes do we have to
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 2 index positions (8 bits)
   ;; So, first, we calculate INDEX/2 (HL/2) to know the target byte, and
   ;;  the remainder to know the group of 4bits we want [ 0000 1111 ], and 
   ;;  that will go to the Carry Flag.
   srl   h             ;; [2]
   rr    l             ;; [2] HL = HL / 2 (HL holds byte offset to advance into the array pointed by DE)
   jr    c, g4b_getGroup1 ;; [2/3] IF Carry, then we want the 4bits in the group 1, 
gb4_getGroup0:
   add  hl, de         ;; [3] HL += DE => HL points to the target byte in the array 
   ld    a, (hl)       ;; [2] A = Target byte
   rrca                ;; [1] <| As we want the grop 0 (4 most significant bits), we rotate them
   rrca                ;; [1]  | 4 times and place them in bits 0 to 3. This makes it easier to
   rrca                ;; [1]  | return the value from 0 to 15, as we only will have to leave this 4
   rrca                ;; [1] <| bits out with and AND
   and   #0x0F         ;; [2] Leave out the 4 bits we wanted
   ld    l, a          ;; [1] Move the return value to L

   ret                 ;; [3] Return to the caller

g4b_getGroup1:
   add  hl, de         ;; [3] HL += DE => HL points to the target byte in the array 
   ld    a, (hl)       ;; [2] A = Target byte
   and   #0x0F         ;; [2] As we want group 1 (bits 0 to 3) we just need to leave these 4 bits out with an AND operation
   ld    l, a          ;; [1] Move the return value to L
   
   ret                 ;; [3] Return to the caller
