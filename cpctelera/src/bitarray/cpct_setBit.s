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

.globl cpct_bitWeights

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
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES (Way 2 for parameter retrieval from stack)              ###
;### MEMORY: 49 bytes (8 table + 41 code)                             ###
;### TIME:                                                            ###
;###   Best Case  (1) = 217 cycles ( 54.25 us)                        ###
;###   Worst Case (0) = 228 cycles ( 57.00 us)                        ###
;########################################################################
;

_cpct_setBit::
   ;; GET Parameters from the stack
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   LD (sb_restoreSP+1), SP  ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                       ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                  ;; [10] AF = Return Address
   POP  DE                  ;; [10] DE = Pointer to the bitarray in memory
   POP  HL                  ;; [10] HL = Index of the bit to be set
   POP  BC                  ;; [10] BC => C = Set Value (0/1), B = Undefined
sb_restoreSP:
   LD SP, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                       ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af                  ;; [10] AF = Return Address
   pop  de                  ;; [10] DE = Pointer to the bitarray in memory
   pop  hl                  ;; [10] HL = Index of the bit to be set
   pop  bc                  ;; [10] BC => C = Set Value (0/1), B = Undefined
   push bc                  ;; [11] Restore Stack status pushing values again
   push hl                  ;; [11] (Interrupt safe way, 6 cycles more)
   push de                  ;; [11]
   push af                  ;; [11]
.endif

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
