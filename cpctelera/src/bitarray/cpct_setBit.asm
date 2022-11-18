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
;;    AF, HL
;;
;; Required memory: 
;;      C-bindings - 36 bytes
;;    ASM-bindings - 32 bytes 
;;
;; Time Measures:
;; (start code)
;; Case       | microSecs(us) | CPU Cycles
;; -------------------------------------------
;;  Any       |       52      |    208
;; -------------------------------------------
;; Asm saving |      -15      |    -60
;; -------------------------------------------
;; (end) 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; We need to know how many bytes do we have to jump into the array, 
   ;; to move HL to that point.  We advance 1 byte for each 8 index 
   ;; positions (8 bits). So, first, we calculate INDEX / 8 (HL/8) to 
   ;; know the target byte. At the same time, we get the index of the bit
   ;; to be tested inside the target byte. That bitnumber is INDEX % 8 (HL % 8),
   ;; so the number formed by the 3 least significant bits of HL. We obtain those
   ;; 3 bits that we will later insert in the middle of the instruction 
   ;; set 0, (HL), which is encoded as 0xCBC6. 0xCB is the prefix (it won't change)
   ;; but 0xC6 == 0b11000110 is encoded as follows:
   ;;    0b1____110 -> Encodes that the instruction is a bit modification to (hl)
   ;;    0b_S______ -> Encodes set(S=1) or reset(S=0). In other words, the bit that will be inserted
   ;;    0b__BBB___ -> The number of the bit to be modified [7-0]. Bit 0 is least significant one (right-most)
   ;;
   ;; So, we will start with 0xC6 and then insert the bit the caller wants to modify
   ;; (3 least significant bits of HL) in BBB. Before that, we will also insert 1 or 0
   ;; in the S bit, depending on the caller wanting Set or Reset operation (insert a 1 or a 0).
   ;; As we start with 0xC6, that would be a SET operation (S=1), so we will change it
   ;; only of the caller wants a RESET (S=0).
   ;; However, we have to take into account that the array considers bits in the reverse 
   ;; order: meaning that bits [0-7] are left-most to right-most, as in the order they appear
   ;; in memory (it is an array). Therefore, we have to convert [0-7] (caller request) to [7-0]
   ;; (instruction encoding). That requires inverting these 3 bits, which is done with CPL instruction.
   ;; Therefore, for better performance, we start with all the bits of the instruction inverted (cpl'd),
   ;; we then insert S and BBB bits as per caller's request, and then we invert all with CPL. 
   ;; So, that means starting with cpl(0xC6) = 0x39. However, as the BBB and S bits are to be inserted
   ;; in the middle of the opcode, we prerotate the instruction 3 bits, to be able to insert
   ;; the bits using right rotations and the Carry. So, cpl(0xC6) = 0x39; rr(0x39, 3) = 0x27.
   ;;
   ld    a, #0x27 ;; [2] A = 0x27 = 0b00100111 => cpl(0xC6) rotated right 3 bits (0xC6 is part of set 0,(hl) instruction, 0xCBC6)
   srl   h        ;; [2] / HL /= 2
   rr    l        ;; [2] \ -- least significant bit goes to carry
   rra            ;; [1] A = 0bB0010011 (Rotate 1 bit right and insert carry)
   srl   h        ;; [2] / HL /= 2
   rr    l        ;; [2] \ -- least significant bit goes to carry
   rra            ;; [1] A = 0bBB001001 (Rotate 1 bit right and insert carry)
   srl   h        ;; [2] / HL /= 2
   rr    l        ;; [2] \ -- least significant bit goes to carry
   rra            ;; [1] A = 0bBBB00100 (Rotate 1 bit right and insert carry)

   ;; Bit 0 of A means set-0/reset-1 (it's reversed). We change it accordingly 
   ;; to what the caller wants (bit 0 of register C. 0=reset, 1=set)
   bit    0, c       ;; [2] Is C odd? (It is expected to be 1 or 0, but only testing the least significant bit)
   jr    nz, set_bit ;; [2/3] If (bit_0_of_C != 0) caller wants to set the bit. Otherwise, wants to reset
reset_bit:
      inc a          ;; [1] Bit 0 of A = 1 ==> Change instruction to reset B, (hl) (reversed)
set_bit:             ;;     A = 0bBBB0010S ==> BBB = bit to set/reset, S = selected set or reset (reversed)

   ;; Now HL contains INDEX / 8 => Number of bytes to add to Array Base to 
   ;; point to the byte where our bit to be tested is.
   rrca           ;; [1] / A = 0bSBBB0010
   rrca           ;; [1] | A = 0b0SBBB001 
   cpl            ;; [1] \ A = 0b1sbbb110 = (0x86 | 0b0sbbb000);
                  ;;     bbb = bit number to be set/reset, s = 1(set) / 0(reset)

   ld (bit_instr), a ;; [4] Modify bit instruction to set/reset our target bit

   add   hl, de   ;; [3] HL += DE ==> Offset + Base ==> HL points to the target byte
bit_instr=.+1
   set   0, (hl)  ;; [2] Set/Reset the required bit (This instruction is automodified)  
   ret            ;; [3] Return 0 otherwise