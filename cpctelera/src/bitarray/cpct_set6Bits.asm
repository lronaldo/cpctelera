;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_set6Bits
;;
;;    Sets the value of a selected element of 6 bits into a bitarray to [0-63].
;;
;; C Definition:
;;    <u8> <cpct_set6Bits> (void* *array*, <u16> *value*, <u16> *index*);
;;
;; Input Parameters (6 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B BC) value - New value [0-63] for the element of 6 bits at the given position. If 
;; you call from assembly, you can safely ignore B register and set only C register.
;;    (2B HL) index - Index of the group of 6 bits in the array to be modified
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_set6Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. So, be warned that giving mistaken values to this function will
;; produce unexpected behaviour (it may change random bits from memory).
;;    * *value* new value for the selected 6-bits element [0-63]. Be sure not to
;; use values greater than 63 or lower than 0. Wrong values can potentially
;; change random bits from values near the selected one.
;;    * *index* position of the 6-bits element to be set in the array, 
;; starting in 0. As this function does not perform any boundary check, if you gave
;; an index outside the boundaries of the array, this function will overwrite a
;; random location in memory, yielding unexpected results.
;;
;; Known limitations:
;;    * Maximum of 65536 elements of 6-bits, 49152 bytes per *array*.      
;;
;; Details:
;;    Set the new *value* of the 6-bits element at the given position (*index*) in the 
;; specified *array*. This function calculates the location in memory considering each 
;; value to take 6-bits in memory. Then, the 6 Least Significant Bits of *value* are 
;; inserted. However, take into account that values greater than 63 or lower than 0 
;; (that is, requiring more than 6 bits) could potentially modify bits from nearby 
;; elements on insertion.   
;; 
;; Examples:
;; (start code)
;;    u8 i;
;;
;;    // Declare and initialize a 6-bit Array using some useful macros
;;    const CPCT_6BITARRAY(my_array, 8) = { 
;;       CPCT_ENCODE6BITS(10, 12, 31, 45),
;;       CPCT_ENCODE6BITS( 7, 60, 18,  2)
;;    };
;; 
;;    // Multiply elements lower than 10 by 2
;;    for(i=0; i < 8; i++) {
;;       u8 value = cpct_get6Bits(my_array, i);
;;       cpct_set6Bits(my_array, value*2, i);
;;    }
;;
;;    // Show all the values. This should print: 10 12 31 45 14 60 18 4
;;    //
;;    for(i=0; i < 8; i++) {
;;       printf("%d ", cpct_get6Bits(my_array, i));
;;    }   
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings   - 94 bytes 
;;    ASM-bindings - 89 bytes
;;
;; Time Measures:
;; (start code)
;; Case      | microSecs (us) | CPU Cycles |
;; -----------------------------------------
;; Best (0)  |      66        |     264    |
;; -----------------------------------------
;; Worst(2)  |      81        |     324    |
;; -----------------------------------------
;; ASM Saving|     -15        |     -60    |
;; -----------------------------------------
;; (end) 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;; We first divide INDEX / 4 because each 4 values are stored in groups of
   ;; 3-bytes. Therefore, this lets us count how many groups of 3-bytes do we
   ;; have to advance to get to the 3-bytes where the target value to be changed is.
   ;; The remainder of the division tells us which one of the 4 values contained
   ;; in that target 3-byte group, contains the target value.
   ;; Each group of 3-bytes stores the 4 values as follows:
   ;;    [00000011][11112222][22333333]
   ;; New number to insert contained in BC is assumed to be [00000000][00xxxxxx]
   ;;
   xor    a       ;; [1] A = 0 
   ld     b, a    ;; [1] B = 0 (Ensure value from B do never produce side effects)
   srl    h       ;; [2] | HL = HL / 4, A = remainder (HL % 4) with bits 1 and 0 inverted in order
   rr     l       ;; [2] |   -- HL holds the number of 3-byte groups to advance from the start
   rla            ;; [1] |      of the array to get into the 3-byte group containing our desired value
   srl    h       ;; [2] |   -- A holds the remainder of division by 4, that tells us which one of
   rr     l       ;; [2] |      the 4 numbers inside the 3-byte group is our desired value.
   rla            ;; [1] \      Warning: this is a 2-bits number, but with both bits inverted in order!

   ;; HL contains the number of 3-byte groups we have to advance to 
   ;; get to the one that contains our desired value. So,
   ;; Multiply HL*3 to know the offset (in bytes) we have to add to DE
   ;; to be at the start of the 3-byte group that contains the desired value
   push  bc       ;; [4] Save BC
   ld     b, h    ;; [1] |BC = HL              (Copy of HL to be used later to multiply by 3)
   ld     c, l    ;; [1] \
   add   hl, hl   ;; [3] HL = 2*HL
   add   hl, bc   ;; [3] HL = 2*HL + HL = 3*HL (Bytes we have to advance to get to the target 3-byte group)
   add   hl, de   ;; [3] HL = 3*HL + DE        (HL now points to the start of the 3-byte group)
   pop   bc       ;; [3] Restore BC

   ;; Now, get the value according to A, which holds the 6-bits group
   rrca           ;; [1]   Rotate A right to get bit 0 into Carry (Most significant bit of remainder)
   jr     c, gs23 ;; [2/3] If Carry, bit was 1, so A contained 2 or 3 (01 or 11, remember bits are in inverse order)
   rrca           ;; [1]   No Carry, So A contains 2 or 4. Rotate again to get next bit and check
   jr    nc, g0   ;; [2/3] If Not Carry, second bit was 0, so A contained a 0 (00)
                  ;; Else, A Contained a 1 (10, remember bits are in inverse order)

;; Target 6-bits value is group 1: (byte 0, bit 1, located 6 bits from the start) 
;;    Target bits = x => [......xx][xxxx....][........]
g1: ;; [29]
   ;; Set up new 6-bits to be inserted in its right place
   ld     a, c    ;; [1] A=6 bits to be inserted [..xxxxxx]
   rlca           ;; [1] / Rotate bits left to put them in their final location: 
   rlca           ;; [1] |   A: [..xxxxxx] >>>  B:A [......xx][xxxx....]
   rla            ;; [1] | We do it rotating 4 times A to the left and
   rl     b       ;; [2] | passing 2 bits to B using carry
   rla            ;; [1] |
   rl     b       ;; [2] \
   
   ;; Insert first 2 bits at the end of the first byte
   ld     c, a    ;; [1] C=A (Save value temporary as we need A)
   ld     a, (hl) ;; [2] A=byte 0 from the target group of 3-bytes, to insert first 2 bits [......xx]
   and #0xFC      ;; [2] Set last 2 bits to 0, leaving the rest untouched, A[______00]
   or     b       ;; [1] Add the first 2 bits from new number of 6-bits in its target location
   ld  (hl), a    ;; [2] Save it to memory

   ;; Insert last 4 bits at the beginning of the second byte
   inc   hl       ;; [2] Point to the 2nd byte from the target 3-bytes group, to insert last 4 bits [xxxx....]
   ld     a, (hl) ;; [2] A=byte 1, where next 4 bits are located A:[xxxx....]
   and #0x0F      ;; [2] Set first 4 bits to 0, leaving the rest untouched, A:[0000____]
   or     c       ;; [1] Add the last 4 bits from the new number of 6-bits in its target location
   ld  (hl), a    ;; [2] Save it to memory
   ret            ;; [3] Return

;; Target 6-bits value is group 0: (byte 0, 6 first bits)  
;;    Target bits = x => [xxxxxx..][........][........]
g0: ;; [14]
   ;; Set up new 6-bits to be inserted in its right place
   rlc    c       ;; [2] | Rotate bits left to put them in their target location: 
   rlc    c       ;; [2] |    C: [..xxxxxx] >>> [xxxxxx..]

   ;; Insert the 6 bits into the first byte of the target 3-byte group
   ld     a, (hl) ;; [2] A=byte 0 from the target group of 3-bytes, to insert the 6-bits in [xxxxxx..]
   and #0x03      ;; [2] Set first 6 bits to 0, leaving latest 2 bits untouched A:[000000__]
   or     c       ;; [1] Add the 6 bits from the new number to their place A[xxxxxx__]
   ld  (hl), a    ;; [2] Save it to memory
   ret            ;; [3] Return

gs23:
   rrca           ;; [1]   Rotate A to get the second bit into Carry
   jr     c, g3   ;; [2/3] If Carry, bit was 1, so A contained 3 (11)
                  ;; Else, A Contained 2 (01, remember bits are in inverse order)

;; Target 6-bits value is group 2: (byte 2, bit 3, located 12 bits from the start)
;;    Target bits = x => [........][....xxxx][xx......]
g2: ;;[29]
   ;; Set up new 6-bits to be inserted in its right place
   xor    a       ;; [1] A=0. Leave it ready to insert 2 of the 6 bits in it
   rr     c       ;; [2] | Rotate bits to the right to put them in their target location
   rra            ;; [1] |    C: [..xxxxxx] >>> C:A [....xxxx][xx......]
   rr     c       ;; [2] |
   rra            ;; [1] \
   ld     b, a    ;; [1] B=A (Save A value for later use, so C:A >> C:B)

   ;; Insert first 4 bits in the second byte of the target 3-byte group
   inc   hl       ;; [2] Point HL to the 2nd byte on the 3-byte group
   ld     a, (hl) ;; [2] A=byte 1 from the target group of 3-bytes, to insert 4-bits in [....xxxx]
   and #0xF0      ;; [2] Set last 4 bits to 0, leaving first 4 bits untouched A:[____0000]
   or     c       ;; [1] Add first 4 bits from the new number to their place A:[____xxxx]
   ld  (hl), a    ;; [2] Save it to memory

   inc   hl       ;; [2] Point HL to the 3rd byte on the 3-byte group
   ld     a, (hl) ;; [2] A=byte 2 from the target group of 3-bytes, to insert 2-bits in [xx......]
   and #0x3F      ;; [2] Set the first 2 bits to 0, leaving last 6 bits untouched A:[00______] 
   or     b       ;; [1] Add last 2 bits from the new number to their place A:[xx______]
   ld  (hl), a    ;; [2] Save it to memory

   ret            ;; [3] Return

;; Target 6-bits value is group 3: (byte 3, bit 5, located 18 bits from the start)
;;    Target bits = x => [........][........][..xxxxxx]
g3: ;; [14]
   ;; Point HL to the 3rd byte of the 3-byte group
   inc   hl       ;; [2] | HL += 2
   inc   hl       ;; [2] \

   ;; Insert the 6 bits at the end of the 3-byte group
   ld     a, (hl) ;; [2] A=byte 2 from the target group of 3-bytes, to insert 6-bits in [..xxxxxx]
   and #0xC0      ;; [2] Set the last 6 bits to 0, leaving first 2 bits untouched A:[__000000]
   or     c       ;; [1] Add last 6 bits from the new number to their place A:[__xxxxxx]
   ld  (hl), a    ;; [2] Save it to memory

   ret            ;; [3] Return