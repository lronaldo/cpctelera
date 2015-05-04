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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setBit
;;
;;    Sets the value of a concrete bit into a bitarray to 0 or 1
;;
;; C Definition:
;;    void <cpct_setBit>  (void* *array*, <u16> *index*, <u8> *value*)
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the bit in the array to be modified
;;    (1B C ) value - New value {0, 1} for the bit at the given position
;;
;; Assembly call (Input parameters on registers):
;;    > call _cpct_setBit_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. Giving and incorrect *array* pointer will have unpredictable
;; results: a random bit from your memory may result changed.
;;    * *index* position of the bit to be retrieved from the array, starting
;; in 0. Again, as this function does not perform any boundary check, if you 
;; gave an index outside the boundaries of the array, a bit outside the array
;; will be changed in memory, what will have unpredictable results.
;;    * *value* new value for the selected bit (0 or 1). Only the Least 
;; Significant Bit (LSB) is used. This means that any given *value* will "work".
;; Odd values will have the same effect as a 1, whereas even values will
;; do the same as a 0.
;;
;; Known limitations:
;;    * Maximum of 65536 bits, 8192 bytes per array.
;;
;; Details:
;;    Set the new *value* of the bit at the given position (*index*) in the 
;; specified *array*. This function assumes that the *array* elements have 
;; a size of 8 bits and also that the given *index* is not bigger than 
;; the number of bits in the *array* (size of the *array* multiplied by 8).         
;; The *value* to be set is also assumed to be 0 or 1, but other values 
;; will "work" (just the least significant bit will be used, so odd values 
;; are treated as 1, even vales as 0).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    52 bytes (8 bytes bitWeights table, 43 bytes code)
;;
;; Time Measures:
;; (start code)
;; Case       | Cycles | microSecs (us)
;; -------------------------------
;; Best  (1)  |   229  |  57.25
;; -------------------------------
;; Worst (0)  |   240  |  60.00
;; -------------------------------
;; Asm saving |   -84  | -21.00
;; -------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


_cpct_setBit::
   ;; GET Parameters from the stack
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   ld (sb_restoreSP+1), sp  ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                       ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                  ;; [10] AF = Return Address
   pop  de                  ;; [10] DE = Pointer to the bitarray in memory
   pop  hl                  ;; [10] HL = Index of the bit to be set
   pop  bc                  ;; [10] BC => C = Set Value (0/1), B = Undefined
sb_restoreSP:
   ld   sp, #0              ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   ei                       ;; [ 4] Enable interrupts again
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

_cpct_setBit_asm::         ;; Entry point for assembly calls using registers for parameter passing

   ld    a, l              ;; [ 4] Save L into A for later use (Knowing which bit to access into the target byte => L % 8)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 8 index positions (8 bits)
   ;; So, first, we calculate INDEX/8 (HL/8) to know the target byte.
   srl   h                 ;; [ 8]
   rr    l                 ;; [ 8]
   srl   h                 ;; [ 8]
   rr    l                 ;; [ 8]
   srl   h                 ;; [ 8]
   rr    l                 ;; [ 8] HL = HL / 8 (Target byte index into the array pointed by DE)
   add  hl, de             ;; [11] HL += DE => HL points to the target byte in the array 

   ;; Calculate which bit will we have to test in the target byte. That will be
   ;; the remainder of INDEX/8, as INDEX/8 represents the byte to access. 
   ;; Knowing which bit to access, we transform it into 2^bitnum, to have its bit weight:
   ;;   a byte with only bitnum bit on, and the rest off. cpct_bitWeights table contains this.
   ld   de, #cpct_bitWeights ;; [10] DE = Pointer to the start of the bitWeights array
   and   #0x07               ;; [ 7] A = L % 8       (bit number to be tested from the target byte of the array) 
   add   e                   ;; [ 4] A += E          (Use bit number as index in cpct_bitweights table, adding it to E (adding to DE starts by adding to E))
   ld    e, a                ;; [ 4] DE = DE + L % 8 (If carry is 0, this already points to the weight of the bit number that is to be tested in the target byte of the array)
   sub   a                   ;; [ 4] A = 0           (Preserving carry flag that must be added to D, if it is 1)
   adc   d                   ;; [ 4] A = D + Carry   (Add Carry to D)
   ld    d, a                ;; [ 4] D += Carry      (Move result to D, to ensure DE points to the bitweight)
   ld    a, (de)             ;; [ 7] A = DE [L % 8]  (bit weight to be tested in the target byte of the array)

   ;; Set/reset the bit of the target byte, using the bit weight stored in A
   bit   0, c              ;; [ 4] Test bit 0 to know if we are setting (1) or resetting (0)
   jp   nz, sb_setBit      ;; [10] If Bit=1, We have to set the bit with OR, else we have to reset it with AND
sb_resetBit:   
   cpl                     ;; [ 4] A = !A (All bits to 1 except the bit we want to reset)
   and (hl)                ;; [ 7] Reset the bit making and AND with only the selected bit to 0
   .db #0x38   ; JR C, xx  ;; [ 7] Fake jumping over OR(HL). Carry is never set after and AND.
sb_setBit:
   or (hl)                 ;; [ 7] Setting the bit with an OR.

   ld (hl), a              ;; [ 7] Saving the new byte in memory, with the bit setted/resetted

   ret                     ;; [10] Return to caller 
