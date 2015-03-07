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
;### FUNCTION: cpct_get4Bits                                          ###
;########################################################################
;### Returns an integer from 0 to 15 depending on the value of the    ###
;### group of 4 bits at the given position in the specified array.    ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of four bits in the array (size of the array multiplied   ###
;### by 2).                                                           ###
;### Limitations: Maximum of 65536 4bit-groups, 32768 bytes per array ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Array Pointer                                            ###
;###  * (2B) Position of the group of four bits in the array          ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  L = integer from 0 to 15                                        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 29 bytes                                                 ###
;### TIME:                                                            ###
;###   Best Case  (1) = 128 cycles ( 32.00 us)                        ###
;###   Worst Case (0) = 151 cycles ( 37.75 us)                        ###
;########################################################################
;
_cpct_get4Bits::

   ;; Get parameters from the stack
   POP   AF            ;; [10] AF = Return address
   POP   DE            ;; [10] DE = Pointer to the array in memory
   POP   HL            ;; [10] HL = Index of the bit we want to get
   PUSH  HL            ;; [11] << Restore stack status
   PUSH  DE            ;; [11]
   PUSH  AF            ;; [11]

   ;; We need to know how many bytes do we have to
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 2 index positions (8 bits)
   ;; So, first, we calculate INDEX/2 (HL/2) to know the target byte, and
   ;;  the remainder to know the group of 4bits we want [ 0000 1111 ], and 
   ;;  that will go to the Carry Flag.
   SRL   H             ;; [ 8]
   RR    L             ;; [ 8] HL = HL / 2 (HL holds byte offset to advance into the array pointed by DE)
   JP C, g4b_getGroup1 ;; [10] IF Carry, then we want the 4bits in the group 1, 
gb4_getGroup0:
   ADD  HL, DE         ;; [11] HL += DE => HL points to the target byte in the array 
   LD   A, (HL)        ;; [ 7] A = Target byte
   RRCA                ;; [ 4] <| As we want the grop 0 (4 most significant bits), we rotate them
   RRCA                ;; [ 4]  | 4 times and place them in bits 0 to 3. This makes it easier to
   RRCA                ;; [ 4]  | return the value from 0 to 15, as we only will have to leave this 4
   RRCA                ;; [ 4] <| bits out with and AND
   AND  #0x0F          ;; [ 7] Leave out the 4 bits we wanted
   LD   L, A           ;; [ 4] Move the return value to L
   RET                 ;; [10] Return to the caller
g4b_getGroup1:
   ADD  HL, DE         ;; [11] HL += DE => HL points to the target byte in the array 
   LD   A, (HL)        ;; [ 7] A = Target byte
   AND  #0x0F          ;; [ 7] As we want group 1 (bits 0 to 3) we just need to leave these 4 bits out with an AND operation
   LD   L, A           ;; [ 4] Move the return value to L
   RET                 ;; [10] Return to the caller
