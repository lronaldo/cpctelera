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
;######################################################################
;### MODULE: Bit Array                                              ###
;### Developed by Alberto García García and Pablo Martínez González ###
;######################################################################
;### This module contains functions to get and set groups of 1, 2   ###
;### and 4 bit in a char array. So data in arrays can be compressed ###
;### in a transparent way to the programmer.                        ###
;######################################################################
;

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
cpct_bitWeights: .db #0x01, #0x02, #0x04, #0x08, #0x10, #0x20, #0x40, #0x80

.globl _cpct_getBit
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

;
;########################################################################
;### FUNCTION: cpct_setBit                                            ###
;########################################################################
;### Set the the value of the bit at the given position in the        ###
;### specified array to a given value (0 or 1).                       ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### bits in the array (size of the array multiplied by 8).           ###
;### The value to set is also asumed to be 0 or 1, but other values   ###
;### will work (just the least significant bit will be used, so odd   ###
;### values are treated as 1, even vales as 0)                        ###
;### Limitations: Maximum of 65536 bits, 8192 bytes per array.        ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B DE) Array Pointer                                         ###
;###  * (2B HL) Index of the bit to be set                            ###
;###  * (1B C)  Value from 0 to 1 to set in the given position        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL	                      ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 54 bytes (8 table + 46 code)                             ###
;### TIME:                                                            ###
;###   Best Case  (1) = 211 cycles ( 52.75 us)                        ###                                                            ###
;###   Worst Case (0) = 222 cycles ( 55.50 us)                        ###                                                            ###
;########################################################################
;
.globl _cpct_setBit
_cpct_setBit::
   ;; GET Parameters from the stack (Pop + Restoring SP)
   LD (sb_restoreSP+1), SP  ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                       ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                  ;; [10] AF = Return Address
   POP  DE                  ;; [10] DE = Pointer to the bitarray in memory
   POP  HL                  ;; [10] HL = Index of the bit to be set
   POP  BC                  ;; [10] BC => C = Set Value (0/1), B = Undefined
sb_restoreSP:
   LD SP, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                       ;; [ 4] Enable interrupts again

   LD  A, L                 ;; [ 4] Save L into A for later use (Knowing which bit to access into the target byte => L % 8)
   
   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 8 index positions (8 bits)
   ;; So, first, we calculate INDEX/8 (HL/8) to know the target byte.
   SRL  H                  ;; [ 8]
   RR   L                  ;; [ 8]
   SRL  H                  ;; [ 8]
   RR   L                  ;; [ 8]
   SRL  H                  ;; [ 8]
   RR   L                  ;; [ 8] HL = HL / 8 (Target byte index into the array pointed by DE)
   ADD  HL, DE             ;; [11] HL += DE => HL points to the target byte in the array 

   ;; Calculate which bit will we have to test in the target byte. That will be
   ;; the remainder of INDEX/8, as INDEX/8 represents the byte to access. 
   ;; Knowing which bit to access, we transform it into 2^bitnum, to have its bit weight:
   ;;   a byte with only bitnum bit on, and the rest off. cpct_bitWeights table contains this.
   LD    DE, #cpct_bitWeights ;; [10] DE = Pointer to the start of the bitWeights array
   AND  #0x07                 ;; [ 7] A = L % 8       (bit number to be tested from the target byte of the array) 
   ADD   E                    ;; [ 4] A += E          (We can do this because we know the vector is 8-byte aligned, and incrementing E by less than 8 will never modify D)
   LD    E, A                 ;; [ 4] DE = DE + L % 8 (Points to the weight of the bit number that is to be tested in the target byte of the array)
   LD    A, (DE)              ;; [ 7] A = DE [L % 8]  (bit weight to be tested in the target byte of the array)

   ;; Set/reset the bit of the target byte, using the bit weight stored in A
   BIT 0, C                ;; [ 4] Test bit 0 to know if we are setting (1) or resetting (0)
   JP NZ, sb_setBit        ;; [10] If Bit=1, We have to set the bit with OR, else we have to reset it with AND
sb_resetBit:   
   CPL                     ;; [ 4] A = !A (All bits to 1 except the bit we want to reset)
   AND (HL)                ;; [ 7] Reset the bit making and AND with only the selected bit to 0
   .db #0x38   ; JR C, xx  ;; [ 7] Fake jumping over OR(HL). Carry is never set after and AND.
sb_setBit:
   OR (HL)                 ;; [ 7] Setting the bit with an OR.

   LD  (HL), A             ;; [ 7] Saving the new byte in memory, with the bit setted/resetted

   RET                     ;; [10] Return to caller 


;
;########################################################################
;### FUNCTION: cpct_get2Bits                                          ###
;########################################################################
;### Returns 0, 1, 2 or 3 depending on the value of the group of 2    ###
;### bits at the given position in the specified array.               ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of two bits in the array (size of the array multiplied    ###
;### by four).                                                        ###
;### Limitations: Maximum of 65536 2bit-groups, 16384 bytes per array ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B DE) Array Pointer                                         ###
;###  * (2B HL) Position of the bit in the array                      ###
;########################################################################
;### RETURN VALUE                                                     ###
;###  L = {0, 1, 2, 3} Value read from 2bitvector                     ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 42 bytes (8 table + 31 code)                             ###
;### TIME:                                                            ###
;###   Best Case  (0) = 171 cycles ( 42.75 us)                        ###
;###   Worst Case (2) = 209 cycles ( 52,25 us)                        ###
;########################################################################
;

.globl _cpct_get2Bits
_cpct_get2Bits::

   ;; Get parameters from the stack
   POP   AF          ;; [10] AF = Return address
   POP   DE          ;; [10] DE = Pointer to the array in memory
   POP   HL          ;; [10] HL = Index of the bit we want to get
   PUSH  HL          ;; [11] << Restore stack status
   PUSH  DE          ;; [11]
   PUSH  AF          ;; [11]

   LD   A, L         ;; [ 4] A = L 
   AND  #0x03        ;; [ 7] A = L % 4 (Calculate the group of 2 bytes we will be getting from the target byte: the remainder of L/4)
   LD   B, A         ;; [ 4] B = index of the group of 2 bits that we want to get from 0 to 3 ([00 11 22 33])

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 4 index positions (8 bits)
   ;; So, first, we calculate INDEX/4 (HL/4) to know the target byte.
   SRL   H           ;; [ 8]
   RR    L           ;; [ 8]
   SRL   H           ;; [ 8]
   RR    L           ;; [ 8] HL = HL / 4 (HL holds byte offset to advance into the array pointed by DE)
   ADD  HL, DE       ;; [11] HL += DE => HL points to the target byte in the array 
   LD    A, (HL)     ;; [ 7] A = array[index] Get the byte where our 2 target bits are located

   ;; Move the 2 required bits to the least significant position (bits 0 & 1)
   ;;   This is done to make easier the opperation of returning a value from 0 to 3 (represented by the 2 bits searched).
   ;;   Once the bits are at least significant position, we only have to AND them and we are done.
   ;; The remainder of INDEX/4 is a value from 0 to 3, representing the index of the 2 bits to be returned
   ;;   inside the target byte [ 0 0 | 1 1 | 2 2 | 3 3 ].
   DEC B               ;; [ 4] If B=0, B-- is negative, turning on S flag
   JP M, g2b_B_is_0    ;; [10] IF S flag is on, B was 0
   DEC B               ;; [ 4] B-- (second time, so B-=2)
   JP P, g2b_B_is_2or3 ;; [10] If S flag is off, B was > 1, else B was 1
g2b_B_is_1:
   RLCA                ;; [ 4] <| Move bits 5 & 4 to positions 0 & 1 with 4 left rotations
   RLCA                ;; [ 4] <|
   JP g2b_B_is_0       ;; [10] We have done 2 rotations, and jump to B_is_0, to make another 2 rotations
g2b_B_is_2or3:
   DEC B               ;; [ 4] B-- (third time, so B-=3)
   JP P, g2b_end       ;; [10] IF S flag is off, B was >2 (3), else B was 2
g2b_B_is_2:
   RRCA                ;; [ 4] <| Move bits 2 & 3 to positions 0 & 1 with 2 right rotations
   RRCA                ;; [ 4] <|
   .db #0xF2 ;JP P, xx ;; [10] Fake jump to gb_end (JP P, xx will be never done, as S is set. Value XX is got from next 2 bytes, which are RLCA;RLCA. Not jumping leaves us 3 bytes from here, at g2b_end)
g2b_B_is_0:
   RLCA                ;; [ 4] <| Move bits 7 & 6 to positions 0 & 1 with 2 left rotations
   RLCA                ;; [ 4] <|
g2b_end:                            ; 0:22, 1:54, 2:60 3:42
   AND #0x03           ;; [ 7] Leave out the 2 required bits (bits 0 & 1, at least significant positions).
   LD   L, A           ;; [ 4] Set the return value in L 

   RET                 ;; [10] Return to caller

;
;########################################################################
;### FUNCTION: cpct_set2Bits                                          ###
;########################################################################
;### Set the the value of the group of 2 bits at the given position   ###
;### in the specified array to a given value.                         ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of four bits in the array (size of the array multiplied   ###
;### by 4). The value to set is also asumed to be lower than 4.       ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B DE) Array Pointer                                         ###
;###  * (2B HL) Position of the group of four bits in the array       ###
;###  * (1B C) Value from 0 to 3  to set in the given position        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL	                      ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  Not computed 	                                                 ###
;########################################################################
;
.globl _cpct_set2Bits
_cpct_set2Bits::
   ;; GET Parameters from the stack (Pop + Restoring SP)
   LD (s2b_restoreSP+1), SP ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                       ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                  ;; [10] AF = Return Address
   POP  DE                  ;; [10] DE = Pointer to the bitarray in memory
   POP  HL                  ;; [10] HL = Index of the bit to be set
   POP  BC                  ;; [10] BC => C = Set Value (0-3), B = Undefined
s2b_restoreSP:
   LD SP, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                       ;; [ 4] Enable interrupts again

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

;
;########################################################################
;### FUNCTION: cpct_get4Bits					                            ###
;########################################################################
;### Returns an integer from 0 to 15 depending on the value of the    ###
;### group of 4 bits at the given position in the specified array.    ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of four bits in the array (size of the array multiplied   ###
;### by 2).                                                           ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Array Pointer                                            ###
;###  * (2B) Position of the group of four bits in the array          ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  A value from 0 to 15 in HL				                      ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  Not computed 	                                                  ###
;########################################################################
;
.globl _cpct_get4Bits
_cpct_get4Bits::

	pop 	af
	pop 	de
	pop 	hl 
	push 	hl 
	push 	de
	push 	af				;; Stack:

	sla 	l 				;; HL <- pos*2 with one left shift.
	rl 		h

	ld		c, l 			;; BC = pos*2+1
	ld		b, h
	inc		bc

	push	hl 				;; Stack: pos*2
	push	bc 				;; Stack: pos*2+1 | pos*2
	push	de 				;; Stack: array | pos*2+1 | pos*2

	call	_cpct_get2Bits	;; HL = bitPair2Val

							;; Stack: array | pos*2+1 | pos*2
	pop		de 				;; Stack: pos*2+1 | pos*2
	pop 	bc 				;; Stack: pos*2
	pop 	bc 				;; Stack:
	push 	hl 				;; Stack: bitPair2Val
	push 	bc 				;; Stack: pos*2 | bitPair2Val
	push 	de 				;; Stack: array | pos*2 | bitPair2Val

	call	_cpct_get2Bits	;; HL = bitPair1Val

							;; Stack: array | pos*2 | bitPair2Val
	pop		de 				;; Stack: pos*2 | bitPair2Val
	pop 	de 				;; Stack: bitPair2Val
	pop 	de 				;; Stack:

	;; DE = bitPair2Val, HL = bitPair1Val

	sla 	l 				;; HL <- bitPair1Val*4 with two left shifts.
	rl 		h
	sla 	l 
	rl 		h

	add 	hl, de 			;; HL <- bitPair1Val*4 + bitPair2Val

	ret


;
;########################################################################
;### FUNCTION: cpct_set4Bits					                      ###
;########################################################################
;### Set the the value of the group of 4 bits at the given position   ###
;### in the specified array to a given value.                         ###
;### It will asume that the array elements have a size of 8 bits and  ###
;### also that the given position is not bigger than the number of    ###
;### groups of four bits in the array (size of the array multiplied   ###
;### by 2). The value to set is also asumed to be lower than 16.      ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Array Pointer                                            ###
;###  * (2B) Position of the group of four bits in the array          ###
;###  * (2B) Value from 0 to 15 to set in the given position          ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL	                      ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  Not computed 	                                                  ###
;########################################################################
;
.globl _cpct_set4Bits
_cpct_set4Bits::

	pop 	af
	pop 	de
	pop 	bc 
	pop 	hl
	push 	hl 
	push 	bc 
	push 	de
	push 	af 				;; Stack:

	push	hl 				;; AF <- value

	srl 	h 				;; HL <- value/4 with two rigth shifts.
	rr 		l
	srl 	h 
	rr 		l

	pop		af
	push 	hl 				;; Stack: value/4

	push 	af 				;; hl <- value 
	pop		hl

	ld 		a, l 			;; HL = value%4
	and 	a, #0x03
	ld 		l, a
	ld 		h, #0x00

	sla 	c 				;; HL <- pos*2 with one left shift.
	rl 		b

	push	bc 				;; Stack: pos*2 | value/4

	inc		bc 				;; BC = pos*2+1

	push	hl 				;; Stack: value%4 | pos*2 | value/4
	push	bc 				;; Stack: pos*2+1 | value%4 | pos*2 | value/4
	push	de 				;; Stack: array | pos*2+1 | value%4 | pos*2 | value/4

	call	_cpct_set2Bits

							;; Stack: array | pos*2+1 | value%4 | pos*2 | value/4
	pop		de 				;; Stack: pos*2+1 | value%4 | pos*2 | value/4
	pop 	bc 				;; Stack: value%4 | pos*2 | value/4
	pop 	bc 				;; Stack: pos*2 | value/4

	push 	de 				;; Stack: array | pos*2 | value/4

	call	_cpct_set2Bits

							;; Stack: array | pos*2 | value/4
	pop		de 				;; Stack: pos*2 | value/4
	pop 	de 				;; Stack: value/4
	pop 	de 				;; Stack:

	ret