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
.module cpct_memutils

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_memset
;;
;;    Fills up a complete byte-array in memory with a given 8-bit value (as std memset)
;;
;; C Definition:
;;    void <cpct_memset> (void* *array*, <u8> *value*, <u16> *size*);
;;
;; Input Parameters (5 Bytes):
;;  (2B DE) array - Pointer to the first byte of the array to be filled up (starting point in memory)
;;  (1B A ) value - 8-bit value to be set
;;  (2B BC) size  - Number of bytes to be set (>= 2)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_memset_asm
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
;;    C-binding   - 14 bytes
;;    ASM-binding -  8 bytes
;;
;; Time Measures: 
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles |
;; ------------------------------------------
;;   Any      |   28 + 6*S     | 112 + 24*S |
;; ------------------------------------------
;; Asm saving |     -18        |    -72     |
;; ------------------------------------------
;; (end code)
;;    S = *size* (Number of total bytes to set)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

   ld  (de), a   ;; [2] Copy the value (A) to the first byte in the array
   dec  bc       ;; [2] BC-- (As 1 byte has alread been copied)
   
   ;; Set up HL and DE for a massive copy of the Value to be set
   ld    h, d    ;; [1] HL = DE (Points to 1st byte of the memory array to be filled up,
   ld    l, e    ;; [1]      ... which already contains the value to be set)
   inc  de       ;; [2] DE++ (Points to the 2nd byte of the memory array to be filled up,
                 ;;          ... where fitst byte will be copied)

   ;; Copy the rest of the bytes
   ldir          ;; [5/6] Copy the reset of the bytes, cloning the first one

   ret           ;; [3] Return  
