;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 Alberto García García
;;  Copyright (C) 2015 Pablo Martínez González
;;  Copyright (C) 2022 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_getBit
;;
;;    Returns the status of a given bit into a bitarray (0 or !0)
;;
;; C Definition:
;;    <u8> <cpct_getBit> (void* *array*, <u16> *index*);
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Position of the bit in the array to be retrieved
;;
;; Assembly call (Input parameters on Registers):
;;    > call cpct_getBit_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. So, be warned that giving mistaken values to this function will
;; not make it fail, but giving an unpredictable return result.
;;    * *index* position of the bit to be retrieved from the array, starting
;; in 0. As this function does not perform any boundary check, if you gave
;; an index outside the boundaries of the array, the return result would
;; be unpredictable and meaningless.
;;
;; Return value:
;;    u8 - Status of the selected bit: *false* (0) when the bit value is 0, 
;; *true* (> 0) when the bit value is 1. Take into account that >0 means any
;; value different than 0, and not necessarily 1.
;;
;; Assembly return values:
;;    A - Status of bit. (A=0) bit=0, (A>0) bit=1
;;    Flag Z - Value of bit. (Z) bit=0, (NZ) bit=1
;;
;; Known limitations:
;;    * Maximum of 65536 bits, 8192 bytes per *array*.      
;;
;; Details:
;;    Returns 0 or >0 depending on the value of the bit at the given 
;; position (*index*) in the specified *array*. It will assume that the 
;; array elements have a size of 8 bits and also that the given position 
;; is not bigger than the number of bits in the array (size of the array 
;; multiplied by 8).
;;
;; Destroyed Register values: 
;;    AF, HL
;;
;; Required memory:
;;      C-bindings - 34 bytes
;;    ASM-bindings - 29 bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | microsec (ms) | Cycles
;; --------------------------------------
;;             C - bindings
;; --------------------------------------
;;  Bit = 1   |      46       |   184
;;  Bit = 0   |      49       |   196
;; --------------------------------------
;;           ASM - bindings
;; --------------------------------------
;;  Bit = 1   |      33       |   162
;;  Bit = 0   |      35       |   170
;; --------------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; We need to know how many bytes do we have to jump into the array, 
   ;; to move HL to that point.  We advance 1 byte for each 8 index 
   ;; positions (8 bits). So, first, we calculate INDEX/8 (HL/8) to 
   ;; know the target byte. At the same time, we get the index of the bit
   ;; to be tested inside the target byte. That bitnumber is INDEX%8, so
   ;; the number formed by the 3 least significant bits. We obtain that
   ;; 3 bits and insert them in the middle of the instruction bit 0, (hl)
   ;; to modify the '0' and put our bitnumber. bit 0, (hl) is 0xCB46.
   ;; However, bit ?, (hl) numbers bits in descending order (7..0) but
   ;; we count them in reverse (ascending order, 0..7) as they are placed
   ;; in memory. So, we need to insert the 3 bits that identify the bitnumber
   ;; and then calculate the complement (cpl) to reverse the order.
   ld    a, #0x37 ;; [2] A = 0x37 = 0b00110111 => cpl(0x46) rotated right 3 bits (0x46 is part of bit 0,(hl) instruction, 0xCB46)
   srl   h        ;; [2] / HL /= 2
   rr    l        ;; [2] \ -- least significant bit goes to carry
   rra            ;; [1] A = 0bC0011011 (Rotate 1 bit right and insert carry)
   srl   h        ;; [2] / HL /= 2
   rr    l        ;; [2] \ -- least significant bit goes to carry
   rra            ;; [1] A = 0bCC001101 (Rotate 1 bit right and insert carry)
   srl   h        ;; [2] / HL /= 2
   rr    l        ;; [2] \ -- least significant bit goes to carry
   rra            ;; [1] A = 0bCCC00110 (Rotate 1 bit right and insert carry)

   ;; Now HL contains INDEX / 8 => Number of bytes to add to Array Base to 
   ;; point to the byte where our bit to be tested is.

   rrca           ;; [1] / A = 0b0CCC0011
   rrca           ;; [1] | A = 0b10CCC001 
   cpl            ;; [1] \ A = 0b01ccc110 = (0x46 | 0b00ccc000); 
                  ;;     CCC = bit number to be tested, ccc = 1's complement of bit number (reverse order)

   ld (bit_instr), a ;; [4] Modify bit instruction to test our target bit

   add   hl, de   ;; [3] HL += DE ==> Offset + Base ==> HL points to the target byte
bit_instr=.+1
   bit   0, (hl)  ;; [2] Test the required bit (This instruction is automodified)
   
   ;; Different returns after bit-testing, depending on C or ASM bindings
