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
;### FUNCTION: cpct_set4Bits                                          ###
;########################################################################
;### Set the the value of the group of 4 bits at the given position   ###
;### in the specified array to a given value.                         ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of four bits in the array (size of the array multiplied   ###
;### by 2).                                                           ###
;### The function expects a value in the range 0-15, but any value    ###
;### greater than that will be cropped (only bits 0-3 will be used)   ###
;### Limitations: Maximum of 65536 4bit-groups, 32768 bytes per array ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B DE) Array Pointer                                         ###
;###  * (2B HL) Position of the group of four bits in the array       ###
;###  * (1B C) Value from 0 to 15 to set in the given position        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL	                      ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 38 bytes                                                 ###
;### TIME:                                                            ###
;###   Best Case  (1) = 169 cycles ( 42.25 us)                        ###
;###   Worst Case (0) = 192 cycles ( 48.00 us)                        ###
;########################################################################
;
_cpct_set4Bits::

   ;; GET Parameters from the stack (Pop + Restoring SP)
   LD (s4b_restoreSP+1), SP ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                       ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                  ;; [10] AF = Return Address
   POP  DE                  ;; [10] DE = Pointer to the bitarray in memory
   POP  HL                  ;; [10] HL = Index of the bit to be set
   POP  BC                  ;; [10] BC => C = Set Value (0-15), B = Undefined
s4b_restoreSP:
   LD SP, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                       ;; [ 4] Enable interrupts again

   LD  A, #0x0F        ;; [ 7] A = 0F  <| Setting up masks for getting upper or lower nibble (4 bits)
   LD  B, #0xF0        ;; [ 7] B = F0  <|
   AND C               ;; [ 4] C = Value to be set (ensure it is in the range 0-15)

   ;; We need to know how many bytes do we have to
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 2 index positions (8 bits)
   ;; So, first, we calculate INDEX/2 (HL/2) to know the target byte, and
   ;;  the remainder to know the group of 4bits we want [ 0000 1111 ], and 
   ;;  that will go to the Carry Flag.
   SRL   H             ;; [ 8]
   RR    L             ;; [ 8] HL = HL / 2 (HL holds byte offset to advance into the array pointed by DE)
   JP C, s4b_setGroup1 ;; [10] IF Carry is set, then last bit of L was 1, we have to set group 1 (bits 0 to 4)
s4b_setGroup0:
   RRCA                ;; [ 4] <| As we are goint to set the grop 0 (4 most significant bits), we rotate the
   RRCA                ;; [ 4]  | value 4 times and place it in bits 4 to 7
   RRCA                ;; [ 4]  |
   RRCA                ;; [ 4] <|
   LD  B, #0x0F        ;; [ 7] We need a diferent mask, as we want to leave out only bits from 0 to 3
s4b_setGroup1:
   ADD  HL, DE         ;; [11] HL += DE => HL points to the target byte in the array 
   LD   C, A           ;; [ 4] C = Value to be set
   LD   A, (HL)        ;; [ 7] A = Target Byte
   AND  B              ;; [ 4] Mask A to reset the bits we want to set
   OR   C              ;; [ 4] Set our value in the 4 bits previously resetted
   LD   (HL), A        ;; [ 7] Save the byte again in the array
   RET                 ;; [10] Return to the caller
