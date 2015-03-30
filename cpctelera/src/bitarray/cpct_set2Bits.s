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

;
;########################################################################
;### FUNCTION: cpct_set2Bits                                          ###
;########################################################################
;### Set the the value of the group of 2 bits at the given position   ###
;### in the specified array to a given value.                         ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of four bits in the array (size of the array multiplied   ###
;### by 4).                                                           ###
;### The function expects a value from 0 to 3. However, if a greater  ###
;### value is given, it will be cropped (only bits 0 & 1 will be used)###
;### Limitations: Maximum of 65536 2bit-groups, 16384 bytes per array ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B DE) Array Pointer                                         ###
;###  * (2B HL) Position of the group of four bits in the array       ###
;###  * (1B C) Value from 0 to 3  to set in the given position        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL	                      ###
;########################################################################
;### MEASURES (Way 2 for parameter retrieval from stack)              ###
;### MEMORY: 64 bytes                                                 ###
;### TIME:                                                            ###
;###   Best Case  (0) = 228 cycles ( 57.00 us)                        ###
;###   Worst Case (1) = 260 cycles ( 65.00 us)                        ###
;########################################################################
;

_cpct_set2Bits::
   ;; GET Parameters from the stack
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   LD (s2b_restoreSP+1), SP ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                       ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                  ;; [10] AF = Return Address
   POP  DE                  ;; [10] DE = Pointer to the bitarray in memory
   POP  HL                  ;; [10] HL = Index of the bit to be set
   POP  BC                  ;; [10] BC => C = Set Value (0-3), B = Undefined
s2b_restoreSP:
   LD SP, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                       ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af                  ;; [10] AF = Return Address
   pop  de                  ;; [10] DE = Pointer to the bitarray in memory
   pop  hl                  ;; [10] HL = Index of the bit to be set
   pop  bc                  ;; [10] BC => C = Set Value (0/3), B = Undefined
   push bc                  ;; [11] Restore Stack status pushing values again
   push hl                  ;; [11] (Interrupt safe way, 6 cycles more)
   push de                  ;; [11]
   push af                  ;; [11]
.endif

   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be set
   ;;   inside the target byte ([ 00 11 22 33 ]).
   LD   A, L                ;; [ 4] A = L (Least significant bits from array index)
   AND  #0x03               ;; [ 7] A = L % 4 (Calculate the group of 2 bytes we will be setting from the target byte: the remainder of L/4)
   LD   B, A                ;; [ 4] B = index of the group of 2 bits that we want to set from 0 to 3 ([00 11 22 33])
   LD   A, C                ;; [ 4] A = C     (Value to be set)
   AND  #0x03               ;; [ 7] A = C % 4 (Ensure value to be set is in the range 0-3)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 4 index positions (8 bits)
   ;; So, first, we calculate INDEX/4 (HL/4) to know the target byte.
   SRL   H           ;; [ 8]
   RR    L           ;; [ 8]
   SRL   H           ;; [ 8]
   RR    L           ;; [ 8] HL = HL / 4 (HL holds byte offset to advance into the array pointed by DE)
   ADD  HL, DE       ;; [11] HL += DE => HL points to the target byte in the array 

   ;; Move the 2 new bits to be set from bits 0 & 1 to their final position in the target byte
   ;;  and setting up a mask to be used for inserting them in the target byte of the array
   DEC B               ;; [ 4] If B=0, B-- is negative, turning on S flag
   JP M, s2b_B_is_0    ;; [10] IF S flag is on, B was 0
   DEC B               ;; [ 4] B-- (second time, so B-=2)
   JP P, s2b_B_is_2or3 ;; [10] If S flag is off, B was > 1, else B was 1
s2b_B_is_1:
   RRCA                ;; [ 4] <| Move bits to positions 5 & 4 using 4 right rotations
   RRCA                ;; [ 4] <|
   LD  C, #0xCF        ;; [ 7] Mask for leaving out bits 5 & 4
   JP s2b_B_is_0 + 2   ;; [10] We have done 2 rotations, and jump to B_is_0 + 2, to make another 2 rotations
s2b_B_is_2or3:
   DEC B               ;; [ 4] B-- (third time, so B-=3)
   JP P, s2b_B_is_3    ;; [10] IF S flag is off, B was >2 (3), else B was 2
s2b_B_is_2:
   RLCA                ;; [ 4] <| Move bits to positions  2 & 3 with 2 left rotations
   RLCA                ;; [ 4] <|
   LD  C, #0xF3        ;; [ 7] Mask for leaving out bits 2 & 3
   JP s2b_end          ;; [10] Done with preparing mask and rotations, continue to setting bits
s2b_B_is_0:
   LD  C, #0x3F        ;; [ 7] Mask for leaving out bits 7 & 6
   RRCA                ;; [ 4] <| Move bits to positions 7 & 6 with 2 right rotations
   RRCA                ;; [ 4] <|
   .db #0xF2 ;JP P, xx ;; [10] Fake jump to gb_end (JP P, xx will be never done, as S is set. Value XX is got from next 2 bytes, which are "LD C, #0xFC". Not jumping leaves us 3 bytes from here, at g2b_end)
s2b_B_is_3:
   LD  C, #0xFC        ;; [ 7] Mask for leaving out bits 1 & 0
s2b_end:
   LD  B, A            ;; [ 4] B = A (Bits to be set)
   LD  A, (HL)         ;; [ 7] A = target Byte
   AND C               ;; [ 4] A = target Byte Masked (Set to 0 the 2 bits we are going to set with our new value)
   OR  B               ;; [ 4] A = final value        (Set the new value for our new bits, ORing them with the other 3 2bit groups)
   LD  (HL), A         ;; [ 7] Store the final value of the target byte in the array

   RET                 ;; [10] Return to caller
