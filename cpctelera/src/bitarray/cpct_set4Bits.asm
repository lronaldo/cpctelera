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
;; Function: cpct_set4Bits
;;
;;    Sets the value of a selected group of 4 bits (nibble) into a bitarray to [0-15]
;;
;; C Definition:
;;    void <cpct_set4Bits>  (void* *array*, <u16> *value*, <u16> *index*)
;;
;; Input Parameters (6 Bytes, B register ignored, only C register is used for value):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the group of 4 bits in the array to be modified
;;    (2B BC) value - New value [0-15] for the group of 4 bits at the given position. If 
;; you call from assembly, you can safely ignore B register and set only C register.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_set4Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. Giving and incorrect *array* pointer will have unpredictable
;; results: a random nibble from your memory may result changed.
;;    * *value* new value for the selected nibble [0-15]. Only the 4 Least 
;; Significant Bits (LSBs) are used. This means that any given *value* 
;; will "work": *value* module 16 will be finally inserted in the *array*
;; position (*index*).
;;    * *index* position of the group of 4 bits (nibble) to be modified in 
;; the array, starting in 0. Again, as this function does not perform any 
;; boundary check, if you gave an index outside the boundaries of the array,
;; a nibble outside the array will be changed in memory, what will have 
;; unpredictable results.
;;
;; Known limitations:
;;    * Maximum of 65536 bits, 32768 bytes per array.
;;
;; Details:
;;    Set the new *value* of the 4-bits group (nibble) at the given position 
;; (*index*) in the specified *array*. This function assumes that the *array* 
;; elements have a size of 8 bits and also that the given *index* is not bigger
;; than the number of nibbles in the *array* (size of the *array* multiplied
;; by 2). The *value* to be set is also assumed to be in the range [0-15] but 
;; other values will "work" (just the 4 Least Significant Bits will be used, 
;; so *value* module 16 will be inserted).
;; 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 28 bytes 
;;   ASM-bindings - 24 bytes
;;
;; Time Measures:
;; (start code)
;; Case       | microSecs(us) | CPU Cycles
;; -----------------------------------------
;; Best  (1)  |       40      |     160 
;; -----------------------------------------
;; Worst (0)  |       45      |     180
;; -----------------------------------------
;; Asm saving |      -15      |     -60
;; -----------------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ld    a, #0x0F      ;; [2] A = 0F  <| Setting up masks for getting upper or lower nibble (4 bits)
   ld    b, #0xF0      ;; [2] B = F0  <|
   and   c             ;; [1] C = Value to be set (ensure it is in the range 0-15)

   ;; We need to know how many bytes do we have to
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 2 index positions (8 bits)
   ;; So, first, we calculate INDEX/2 (HL/2) to know the target byte, and
   ;;  the remainder to know the group of 4bits we want [ 0000 1111 ], and 
   ;;  that will go to the Carry Flag.
   srl   h             ;; [2]
   rr    l             ;; [2] HL = HL / 2 (HL holds byte offset to advance into the array pointed by DE)
   jr    c, s4b_setGroup1 ;; [2/3] IF Carry is set, then last bit of L was 1, we have to set group 1 (bits 0 to 4)
s4b_setGroup0:
   rrca                ;; [1] <| As we are going to set the group 0 (4 most significant bits), we rotate the
   rrca                ;; [1]  | value 4 times and place it in bits 7 to 4
   rrca                ;; [1]  |
   rrca                ;; [1] <|
   ld    b, #0x0F      ;; [2] We need a diferent mask, as we want to leave out only bits from 0 to 3
s4b_setGroup1:
   add  hl, de         ;; [3] HL += DE => HL points to the target byte in the array 
   ld   c, a           ;; [1] C = Value to be set
   ld   a, (hl)        ;; [2] A = Target Byte
   and  b              ;; [1] Mask A to reset the bits we want to set
   or   c              ;; [1] Set our value in the 4 bits previously resetted
   ld   (hl), a        ;; [2] Save the byte again in the array
   ret                 ;; [3] Return to the caller
