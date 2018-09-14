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
;; Function: cpct_setBit
;;
;;    Sets the value of a concrete bit into a bitarray to 0 or 1
;;
;; C Definition:
;;    void <cpct_setBit>  (void* *array*, <u16> *value*, <u16> *index*)
;;
;; Input Parameters (6 Bytes, B register ignored, only C register is used for value):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the bit in the array to be modified
;;    (2B BC) value - New value {0, 1} for the bit at the given position.  If 
;; you call from assembly, you can safely ignore B register and set only C register.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setBit_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. Giving and incorrect *array* pointer will have unpredictable
;; results: a random bit from your memory may result changed.
;;    * *value* new value for the selected bit (0 or 1). Only the Least 
;; Significant Bit (LSB) is used. This means that any given *value* will "work".
;; Odd values will have the same effect as a 1, whereas even values will
;; do the same as a 0.
;;    * *index* position of the bit to be retrieved from the array, starting
;; in 0. Again, as this function does not perform any boundary check, if you 
;; gave an index outside the boundaries of the array, a bit outside the array
;; will be changed in memory, what will have unpredictable results.
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
;;      C-bindings - 39 bytes
;;    ASM-bindings - 35 bytes 
;;      bitWeights - +8 bytes vector required by both bindings. Take into 
;; account that this vector is included only once if you use different 
;; functions referencing to it.
;;
;; Time Measures:
;; (start code)
;; Case       | microSecs(us) | CPU Cycles
;; -------------------------------------------
;; Best  (1)  |       55      |    220
;; -------------------------------------------
;; Worst (0)  |       57      |    228
;; -------------------------------------------
;; Asm saving |      -15      |    -60
;; -------------------------------------------
;; (end) 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_bitWeights

   ld    a, l              ;; [1] Save L into A for later use (Knowing which bit to access into the target byte => L % 8)

   ;; We need to know how many bytes do we have to 
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 8 index positions (8 bits)
   ;; So, first, we calculate INDEX/8 (HL/8) to know the target byte.
   srl   h                 ;; [2]
   rr    l                 ;; [2]
   srl   h                 ;; [2]
   rr    l                 ;; [2]
   srl   h                 ;; [2]
   rr    l                 ;; [2] HL = HL / 8 (Target byte index into the array pointed by DE)
   add  hl, de             ;; [3] HL += DE => HL points to the target byte in the array 

   ;; Calculate which bit will we have to test in the target byte. That will be
   ;; the remainder of INDEX/8, as INDEX/8 represents the byte to access. 
   ;; Knowing which bit to access, we transform it into 2^bitnum, to have its bit weight:
   ;;   a byte with only bitnum bit on, and the rest off. cpct_bitWeights table contains this.
   ld   de, #cpct_bitWeights ;; [3] DE = Pointer to the start of the bitWeights array
   and   #0x07               ;; [2] A = L % 8       (bit number to be tested from the target byte of the array) 
   add   e                   ;; [1] A += E          (Use bit number as index in cpct_bitweights table, adding it to E) 
                             ;; ....                 (adding to DE starts by adding to E)
   ld    e, a                ;; [1] DE = DE + L % 8 (If carry is 0, this already points to the weight of 
                             ;; ....                 the bit number that is to be tested in the target byte of the array)
   sub   a                   ;; [1] A = 0           (Preserving carry flag that must be added to D, if it is 1)
   adc   d                   ;; [1] A = D + Carry   (Add Carry to D)
   ld    d, a                ;; [1] D += Carry      (Move result to D, to ensure DE points to the bitweight)
   ld    a, (de)             ;; [2] A = DE [L % 8]  (bit weight to be tested in the target byte of the array)

   ;; Set/reset the bit of the target byte, using the bit weight stored in A
   bit   0, c              ;; [2]  Test bit 0 to know if we are setting (1) or resetting (0)
   jr   nz, sb_setBit      ;; [2/3] If Bit=1, We have to set the bit with OR, else we have to reset it with AND
sb_resetBit:   
   cpl                     ;; [1] A = !A (All bits to 1 except the bit we want to reset)
   and (hl)                ;; [2] Reset the bit making and AND with only the selected bit to 0
   .db #0x38   ; JR C, xx  ;; [2] Fake jumping over OR(HL). Carry is never set after and AND.
sb_setBit:                 ;;     So, this jump is never executed, effectively "jumping over" or(hl)
   or (hl)                 ;; [2] Setting the bit with an OR.

   ld (hl), a              ;; [2] Saving the new byte in memory, with the bit setted/resetted

   ret                     ;; [3] Return to caller 
