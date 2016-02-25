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
;; Function: cpct_get6Bits
;;
;;    Returns the value ( [0-63] ) of a given group of 6 bits into an array 
;;
;; C Definition:
;;    <u8> <cpct_get6Bits> (void* *array*, <u16> *index*);
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) array - Pointer to the first byte of the array
;;    (2B HL) index - Index of the group of 6 bits to be retrieved from the array
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_get6Bits_asm
;;
;; Parameter Restrictions:
;;    * *array* must be the memory location of the first byte of the array.
;; However, this function will accept any given 16-value, without performing
;; any check. So, be warned that giving mistaken values to this function will
;; not make it fail, but return an unpredictable value.
;;    * *index* position of the group of 6 bits to be retrieved from the array, 
;; starting in 0. As this function does not perform any boundary check, if you gave
;; an index outside the boundaries of the array, the return result would
;; be unpredictable and meaningless.
;;
;; Return value:
;;    u8 - Value of the selected group of 6 bits: [0-63]
;;
;; Known limitations:
;;    * Maximum of 65536 groups of 6-bits, 49152 bytes per *array*.      
;;
;; Details:
;;    Returns a value from 0 to 63 depending on the value of the 6-bits group at 
;; the given position (*index*) in the specified *array*. It will treat memory
;; at *array* location as if it contained consecutive 6-bits values instead of bytes.
;; The function internally does the required calculations to get 6-bits groups.
;; 
;; Examples:
;; (start code)
;;    // 6-bit Array containing values 63, 0, 63, 0, 63, 0
;;    const u8 my_array[6] = { 0b11111100, 0b00001111, 0b11000000, 0b11111100, 0b00001111, 0b11000000 };
;; 
;;    // Get the 1st and 4th value
;;    u8 value1 = cpct_get6Bits(my_array, 0);
;;    u8 value4 = cpct_get6Bits(my_array, 3);
;;    printf("Values obtained: %d %d", value1, value4);   // This will print 63, 0
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings   - 25 bytes 
;;    ASM-bindings - 22 bytes
;;
;; Time Measures:
;; (start code)
;; Case      | microSecs (us) | CPU Cycles |
;; -----------------------------------------
;; Best (0)  |      37        |     148    |
;; -----------------------------------------
;; Worst(1|3)|      49        |     196    |
;; -----------------------------------------
;; ASM Saving|     -12        |     -48    |
;; -----------------------------------------
;; (end)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;; We first divide INDEX / 4 because each 4 values are stored in groups of
   ;; 3-bytes. Therefore, this let us count how many groups of 3-bytes do we
   ;; have to advance to get to the 3-bytes where our desired value is stored.
   ;; The remainder of the division tells us which one of the 4 values contained
   ;; in that target 3-byte group, contains the desired value.
   ;; Each group of 3-bytes stores the 4 values as follows:
   ;;    [00000011][11112222][22333333]
   ;;
   xor   a             ;; [1] A = 0 
   srl   h             ;; [2] | HL = HL / 4, A = HL % 4
   rr    l             ;; [2] |   -- HL holds the 3-byte offset to advance into the array pointed by DE
   rlca                ;; [1] |   -- A  holds the remainder of division by 4, which is the group
   srl   h             ;; [2] |      number inside the 3-bytes that contain the number
   rr    l             ;; [2] |
   rlca                ;; [1] \

   ;; HL contains the number of 3-byte groups we have to advance to 
   ;; get to the one that contains our desired value. So,
   ;; Multiply HL*3 to know the offset (in bytes) we have to add to DE
   ;; to be at the start of the 3-byte group that contains the desired value
   ld    b, h          ;; [1] BC = HL
   ld    c, l          ;; [1]
   add  hl, hl         ;; [3] HL = 2*HL
   add  hl, bc         ;; [3] HL = 3*HL
   add  hl, de         ;; [3] HL = 3*HL + DE (Points to the start of the 3-byte group)

   ;; Now, get the value according to A, which holds the 6-bits group
   rrca           ;; [1]   Rotate A right to get bit 0 into Carry
   jr     c, gs13 ;; [2/3] If Carry, bit was 1, so A contained 1 or 3
   rrca           ;; [1]   No Carry, So A contains 2 or 4. Rotate again to get next bit and check
   jr     c, g2   ;; [2/3] If Carry, second bit was 1, so A contained a 2
                  ;; Else, A Contained a 0

;; Desired 6-bits value is group 0: (byte 0, 6 first bits)  
;;    Desired bits = x => [xxxxxx..][........][........]
g0:
   ld     l, (hl) ;; [2] L = byte containing desired 6 bits
   srl    l       ;; [2] | Desired 6 bits are the first 6 bits of L...
   srl    l       ;; [2] | ... shift them left to make L=desired value
   ret            ;; [3] Return

;; Desired 6-bits value is group 2: (byte 2, bit 3, located 12 bits from the start)
;;    Desired bits = x => [........][....xxxx][xx......]
g2:
   inc   hl       ;; [2] Point HL to byte 1, which contains first 4 bits of desired value
   ld     a, (hl) ;; [2] A=byte 1
   and    #0x0F   ;; [2] Remove first 4 bits of the byte: they're not from the desired value
   ld     b, a    ;; [1] Save these 4 bits into B
   inc   hl       ;; [2] Point HL to byte 2, which has last 2 bits
   ld     a, (hl) ;; [2] A=byte 2
   and    #0xC0   ;; [2] Remove last 6 bits, as we only want the first 2
   or     b       ;; [1] Add up the 4 bits and the other 2. They will be like this [oo..xxxx]
   rlca           ;; [1] / The 2 oo bits should be the last, so we have to rotate them 2 times
   rlca           ;; [1] \ to leave it as [..xxxxoo]
   ld     l, a    ;; [1] Move value to L for returning it
   ret            ;; [3] Return

gs13:
   rrca           ;; [1]   Rotate A to get the second bit into Carry
   jr     c, g3   ;; [2/3] If Carry, bit was 1, so A contained 3
                  ;; Else, A Contained 1

;; Desired 6-bits value is group 1: (byte 0, bit 1, located 6 bits from the start) 
;;    Desired bits = x => [......xx][xxxx....][........]
g1:
   ld     b, (hl) ;; [2] B=byte 0, where first 2 bits are located B:[......oo]
   inc   hl       ;; [2] Point HL to the byte 1
   ld     a, (hl) ;; [2] A=byte 1, where next 4 bits are located A:[xxxx....]
   srl    b       ;; [2] Shift B right and move last bit into the Carry
   rra            ;; [1] rotate A right and insert the Carry as leftmost bit B:[.......o] A:[oxxxx...]
   srl    b       ;; [2] Repeat both previous operations...
   rra            ;; [1] ... leaving B:[........] A:[ooxxxx..]
   srl    a       ;; [2] | Shift A to more times to the right, to 
   srl    a       ;; [2] | get the final value A:[..ooxxxx]
   ld     l, a    ;; [1] Move return value to L for returning it
   ret            ;; [3] Return

;; Desired 6-bits value is group 3: (byte 3, bit 5, located 18 bits from the start)
;;    Desired bits = x => [........][........][..xxxxxx]
g3:
   inc   hl       ;; [2] | HL += 2, to make HL point to byte 2
   inc   hl       ;; [2] \
   ld     a, (hl) ;; [2] A = byte 2, which contains the desired value
   and    #0x3F   ;; [2] Remove 2 leftmost bits that are not part of the desired value
   ld     l, a    ;; [1] Move return value to L for returning it
   ret            ;; [3] ret