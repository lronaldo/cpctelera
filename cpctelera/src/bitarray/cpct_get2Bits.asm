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
;;    It also stores a pointer to the byte that has been accessed (the byte
;; where the 2-bits element was) and the 2-bit index of the element (0, 1, 2, 3)
;; inside the accessed byte. These two values are used by <cpct_getNext2Bits>
;; function to access the next 2 bits in a faster way.
;;
;; Destroyed Register values: 
;;    AF, DE, HL
;;
;; Required memory:
;;    C-binding   - 48 bytes
;;    ASM-binding - 42 bytes
;;
;; Time Measures: 
;; (start code)
;; Case       | Microsecs | CPU Cycles
;; -----------------------------------
;; Best (3)   |    44     |   176
;; -----------------------------------
;; Worst (1)  |    48     |   192
;; -----------------------------------
;; ASM-saving |   -21     |   -44
;; -----------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;
;; Start of the function code (without bindings for calling)
;;

   ;; Calculate the group of 2 bits we are selecting and setting up the
   ;; jump to the code that selects this 2 bits for returning the appropriate
   ;; value. The group of 2 bits could be 0, 1, 2 or 3, and is determined by
   ;; the last 2 bits of the index of the element (HL % 4). 
   ;; Once the group is calculated, we invert the bit 0 to reorder the 
   ;; options this way [1, 0, 3, 2] which is optimal for making the code
   ;; of each option take only two bytes of memory. 
   ;; Finally, as each option takes 2 bytes of memory, we multiply the 
   ;; calculated option by 2 (rlca) to get the size of the jump to the 
   ;; code for the selected option.
   ;;
   ld    a, l                  ;; [1] A = L     
   and   #0x03                 ;; [2] A = A % 4 (A = Index of the group of two bytes [0,1,2,3])
   ld   (g2b_bitGroupIdx), a   ;; [4] Save Group Index for function "getNext2Bits"
   xor   #0x01                 ;; [2] Change the group order to [1, 0, 3, 2] by manipulating the index
   rlca                        ;; [1] A = 2A    (Size in bytes of the jump to the code that moves the bits)
   ld   (g2b_jump_select+1), a ;; [4] Place the size in bytes in the JR instruction that jumps to the code

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
   ld   (g2b_elementPtr), hl ;; [5] Save Byte Pointer to the element being accessed for getNext2Bits function

   ;; Move the 2 required bits to the least significant position (bits 0 & 1)
   ;;   This is done to make easier the opperation of returning a value from 0 to 3 (represented by the 2 bits searched).
   ;;   Once the bits are at least significant position, we only have to AND them and we are done.
   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be returned
   ;;   inside the target byte [ 0 0 | 1 1 | 2 2 | 3 3 ].  
g2b_jump_select:
   jr g2b_end     ;; [3] Jump to the required operation for isolating the 2 desired bits.
                  ;; ... The operand of this instruction is modified by previous code.

   ;; 4 options, 2 bytes per option
g2b_bits_54:
   rlca           ;; [1] Option 1: Bits 54 moved to 10 using 4 RLCA's
   rlca           ;; [1]
g2b_bits_76:
   rlca           ;; [1] Option 0: Bits 76 moved to 10 using 2 RLCA's
   rlca           ;; [1]
g2b_bits_10:
   jr g2b_end     ;; [3] Option 3: Nothing to do, as we are returning bits 10,
                  ;; ... so jump to the end
g2b_bits_32:
   rrca           ;; [1] Option 2: Bits 32 moved to 10 using 2 RRCA's
   rrca           ;; [1]
g2b_end:           
   and   #0x03    ;; [2] Leave out the 2 required bits (bits 0 & 1, at least significant positions).
   ld    l, a     ;; [1] Set the return value in L 

   ret            ;; [3] Return to caller
   
;; Variables to be used to access the next 2 bits faster
g2b_elementPtr::  .dw 0000
g2b_bitGroupIdx:: .db 00