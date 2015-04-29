;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_memset

.include /sprites.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_memset
;;
;;    Fills up a complete byte-array in memory with a given 8-bit value (as std memset)
;;
;; C Definition:
;;    void <cpct_memset> (void* *array*, <u16> *size*, <u8> *value*);
;;
;; Input Parameters (5 Bytes):
;;  (2B DE) array - Pointer to the first byte of the array to be filled up (starting point in memory)
;;  (2B BC) size  - Number of bytes to be set (>= 2)
;;  (1B A ) value - 8-bit value to be set
;;
;; Parameter Restrictions:
;;  * *array* could theoretically be any 16-bit memory location. However, take into 
;; account that this function does no check at all, and you could mistakenly overwrite 
;; important parts of your program, the screen, the firmware... Use it with care.
;;  * *size* must be greater than 1. It represents the size of the array, or the number 
;; of total bytes that will be set to the *value*. As this function starts moving 1 
;; first byte and then cloning it, the minimum amount *size* should be 2. Beware! 
;; *Sizes 0 and 1* can cause this to *overwrite the entire memory*. 
;;  * *value* could be any 8-bit value, without restrictions.
;;
;; Details:
;;    Sets all the bytes of an *array* in memory to the same given *value*. This is the
;; same operation as std memset, from the standard C library does. However, this function
;; is much faster than std C memset, so it is recommended for your productions. The 
;; technique this function uses to be so faster is as follows:
;;
;;  1 - Sets up the first byte of the *array* to the *value*
;;  2 - Makes HL point to the first byte and DE to the second
;;  3 - BC has the total bytes to copy minus 1 (the first already set)
;;  4 - LDIR copies first byte into second, then second into third...
;;
;;    This function works for array sizes from 2 to 65535 (it does not work for 0 or 1).
;; However, it is recommended that you use it for values greater than 2. Depending on
;; your code, using memset for values in the range [2-8] could underperform simple
;; variable assignments. 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    20 bytes
;;
;; Time Measures:
;; (start code)
;; Case   |   Cycles   | microSecs (us)
;; -------------------------------------
;; Any    | 108 + 21*S | 27.00 + 5.25*S
;; -------------------------------------
;; (end code)
;;    S = *size* (Number of total bytes to set)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

_cpct_memset::
   ;; Recover parameters from stack
   ld   hl, #2       ;; [10] Make HL point to the byte where parameters start in the
   add  hl, sp       ;; [11] ... stack (first 2 bytes are return address)
   ld    e, (hl)     ;; [ 7] DE = Pointer to first byte in memory for memset
   inc  hl           ;; [ 6]
   ld    d, (hl)     ;; [ 7] 
   inc  hl           ;; [ 6]
   ldi               ;; [16] (HL)->(DE) Copy value to the first byte of the memory to be set
                     ;; .... and, at the same time, do INC HL, INC DE and DEC BC
   ld    c, (hl)     ;; [ 7] BC = Amount of bytes in memory to set to the value of A
   inc  hl           ;; [ 6]
   ld    b, (hl)     ;; [ 7]
   dec   bc          ;; [ 6] BC-- (As 1 byte has alread been copied)

   ;; Set up HL and DE for a massive copy of the Value to be set
   ld    h, d        ;; [ 4] HL = DE (2nd byte of the memory array to be filled up)
   ld    l, e        ;; [ 4] 
   dec   hl          ;; [ 6] HL-- (Point to the first position of the memory array to be
                     ;; .... filled up, which already contains the value to be set)

   ;; Copy the rest of the bytes
   ldir              ;; [21/16] Copy the reset of the bytes, cloning the first one

   ret               ;; [10] Return  
