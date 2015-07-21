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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_set4Bits
;;
;;    Sets the value of a selected group of 4 bits (nibble) into a bitarray to [0-15]
;;
;; C Definition:
;;    void <cpct_set4Bits>  (void* *array*, <u16> *index*, <u8> *value*)
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the group of 4 bits in the array to be modified
;;    (1B C ) value - New value [0-15] for the group of 4 bits at the given position
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_set4Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. Giving and incorrect *array* pointer will have unpredictable
;; results: a random nibble from your memory may result changed.
;;    * *index* position of the group of 4 bits (nibble) to be modified in 
;; the array, starting in 0. Again, as this function does not perform any 
;; boundary check, if you gave an index outside the boundaries of the array,
;; a nibble outside the array will be changed in memory, what will have 
;; unpredictable results.
;;    * *value* new value for the selected nibble [0-15]. Only the 4 Least 
;; Significant Bits (LSBs) are used. This means that any given *value* 
;; will "work": *value* module 16 will be finally inserted in the *array*
;; position (*index*).
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
;;    33 bytes 
;;
;; Time Measures:
;; (start code)
;; Case       | Cycles | microSecs (us)
;; -------------------------------
;; Best  (1)  |   175  |  43.75
;; -------------------------------
;; Worst (0)  |   198  |  49.50
;; -------------------------------
;; Asm saving |   -84  | -21.00
;; -------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_set4Bits::
   ;; GET Parameters from the stack (Pop + Restoring SP)
   pop  af                  ;; [10] AF = Return Address
   pop  de                  ;; [10] DE = Pointer to the bitarray in memory
   pop  hl                  ;; [10] HL = Index of the bit to be set
   pop  bc                  ;; [10] BC => C = Set Value (0/15), B = Undefined
   push bc                  ;; [11] Restore Stack status pushing values again
   push hl                  ;; [11] (Interrupt safe way, 6 cycles more)
   push de                  ;; [11]
   push af                  ;; [11]

cpct_set4Bits_asm::    ;; Entry point for assembly calls using registers for parameter passing

   ld    a, #0x0F      ;; [ 7] A = 0F  <| Setting up masks for getting upper or lower nibble (4 bits)
   ld    b, #0xF0      ;; [ 7] B = F0  <|
   and   c             ;; [ 4] C = Value to be set (ensure it is in the range 0-15)

   ;; We need to know how many bytes do we have to
   ;; jump into the array, to move HL to that point.
   ;; We advance 1 byte for each 2 index positions (8 bits)
   ;; So, first, we calculate INDEX/2 (HL/2) to know the target byte, and
   ;;  the remainder to know the group of 4bits we want [ 0000 1111 ], and 
   ;;  that will go to the Carry Flag.
   srl   h             ;; [ 8]
   rr    l             ;; [ 8] HL = HL / 2 (HL holds byte offset to advance into the array pointed by DE)
   jp    c, s4b_setGroup1 ;; [10] IF Carry is set, then last bit of L was 1, we have to set group 1 (bits 0 to 4)
s4b_setGroup0:
   rrca                ;; [ 4] <| As we are goint to set the grop 0 (4 most significant bits), we rotate the
   rrca                ;; [ 4]  | value 4 times and place it in bits 4 to 7
   rrca                ;; [ 4]  |
   rrca                ;; [ 4] <|
   ld    b, #0x0F      ;; [ 7] We need a diferent mask, as we want to leave out only bits from 0 to 3
s4b_setGroup1:
   add  hl, de         ;; [11] HL += DE => HL points to the target byte in the array 
   ld   c, a           ;; [ 4] C = Value to be set
   ld   a, (hl)        ;; [ 7] A = Target Byte
   and  b              ;; [ 4] Mask A to reset the bits we want to set
   or   c              ;; [ 4] Set our value in the 4 bits previously resetted
   ld   (hl), a        ;; [ 7] Save the byte again in the array
   ret                 ;; [10] Return to the caller
