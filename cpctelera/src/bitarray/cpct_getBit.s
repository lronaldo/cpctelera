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

;
;########################################################################
;### FUNCTION: cpct_getBit                                            ###
;########################################################################
;### Returns 0 or >0 depending on the value of the bit at the given   ###
;### position in the specified array.                                 ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### bits in the array (size of the array multiplied by eight).       ###
;### Limitations: Maximum of 65536 bits, 8192 bytes per array.        ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B DE) Array Pointer                                         ###
;###  * (2B HL) Position of the bit in the array                      ###
;########################################################################
;### RETURN VALUE                                                     ###
;###  L = 0 if bit was unset                                          ###
;###  L > 0 if bit was set                                            ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 39 bytes (8 table + 31 code)                             ###
;### TIME: 179 cycles ( 44.75 us)                                     ###
;########################################################################
;

.bndry 8 ;; Make this vector start at a 8-byte aligned address to be able to use 8-bit arithmetic with pointers
cpct_bitWeights:: .db #0x01, #0x02, #0x04, #0x08, #0x10, #0x20, #0x40, #0x80

_cpct_getBit::

   ;; Get parameters from the stack
   POP   AF          ;; [10] AF = Return address
   POP   DE          ;; [10] DE = Pointer to the array in memory
   POP   HL          ;; [10] HL = Index of the bit we want to get
   PUSH  HL          ;; [11] << Restore stack status
   PUSH  DE          ;; [11]
   PUSH  AF          ;; [11]

   ;; We only access bytes at once in memory. We need to calculate which
   ;; bit will we have to test in the target byte of our array. That will be
   ;; the remainder of INDEX/8, as INDEX/8 represents the byte to access.
   LD    BC, #cpct_bitWeights ;; [10] BC = Pointer to the start of the bitWeights array
   LD    A, L                 ;; [ 4]
   AND  #0x07                 ;; [ 7] A = L % 8       (bit number to be tested from the target byte of the array) 
   ADD   C                    ;; [ 4] A += C          (We can do this because we know the vector is 8-byte aligned, and incrementing C by less than 8 will never modify B)
   LD    C, A                 ;; [ 4] BC = BC + L % 8 (Points to the weight of the bit number that is to be tested in the target byte of the array)
   LD    A, (BC)              ;; [ 7] A = BC [L % 8]  (bit weight to be tested in the target byte of the array)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 8 index positions (8 bits)
   ;; So, first, we calculate INDEX/8 (HL/8) to know the target byte.
   SRL  H            ;; [ 8]
   RR   L            ;; [ 8]
   SRL  H            ;; [ 8]
   RR   L            ;; [ 8]
   SRL  H            ;; [ 8]
   RR   L            ;; [ 8] HL = HL / 8 (Target byte index into the array pointed by DE)

   ;; Reach the target byte and test the bit using the bit weight stored in A
   ADD  HL, DE       ;; [11] HL += DE => HL points to the target byte in the array 
   AND  (HL)         ;; [ 7] Test the selected bit in the target byte in the array
   LD   L, A         ;; [ 4] Return value (0 if bit is not set, !=0 if bit is set)

   RET               ;; [10] Return to caller
