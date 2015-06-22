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

.include /memutils.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_memset_f8
;;
;;    Fills up a complete array in memory setting bytes 2-by-2, in chuncks of 
;; 8 bytes. Size of the array must be multiple of 8.
;;
;; C Definition:
;;    void <cpct_memset_f8> (void* *array*, <u16> *value*, <u16> *size*);
;;
;; Warning:
;;    * This function disables interrupts while operating and uses the stack
;; pointer. Take it into account when you require interrupts or the stack
;; pointer. When in doubt, use <cpct_memset> instead.
;;    * At the end of the function, it reenables interrupts, no matter how
;; they were before. Please, take into account.
;;
;; Input Parameters (5 Bytes):
;;  (2B HL) array - Pointer to the first byte of the array to be filled up (starting point in memory)
;;  (2B BC) size  - Number of bytes to be set (>= 8, multiple of 8)
;;  (2B DE) value - 16-bit value to be set (Pair of bytes)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_memset_f8_asm
;;
;; Parameter Restrictions:
;;  * *array* could theoretically be any 16-bit memory location. However, take into 
;; account that this function does no check at all, and you could mistakenly overwrite 
;; important parts of your program, the screen, the firmware... Use it with care.
;;  * *size* must be greater than 7 and multiple of 8. It represents the size of the 
;; array, or the number of total bytes that will be set to the *value*. This function 
;; sets bytes 2-by-2, in chuncks of 8 bytes, so the minimum amount of bytes to be set is 8. 
;; *Beware!* sizes below 8 can cause this function to *overwrite the entire memory*. 
;;  * *value* could be any 16-bit value, without restrictions. It is considered as 
;; a pair of bytes that will be copied to every 2-bytes in the array.
;;
;; Details:
;;    Sets all pairs of bytes of an *array* in memory to the same given *value*. This is 
;; the same operation as std memset from standard C library, but with the added advantage
;; of being faster and letting the user define the contents 16-bits-by-16-bits instead of
;; 8. The technique used by this function is as follows:
;;
;;  1 - It saves the value of SP to recover it at the end of the function
;;  2 - It places SP at the last 2-bytes of the array
;;  3 - It uses PUSH instructions to set bytes 2-by-2, in chuncks of 8 bytes, until the entire array is set
;;
;;    This function works for array sizes from 2 to 65528. However, it is recommended 
;; that you use it for values much greater than 8. Depending on your code, using <cpct_memset_f> 
;; for values in the range [8-16] could underperform simple variable assignments. 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    25 bytes
;;
;; Time Measures: 
;; (start code)
;;   Case     |   Cycles        | microSecs (us)
;; -----------------------------------------
;;   Any      | 153 + 60*(BC/8) | 38,25 + 15*BC
;; -----------------------------------------
;; Asm saving |    -30          |   -7.50
;; -----------------------------------------
;; (end code)
;;    S = *size* (Number of total bytes to set)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

;; Assembly entry point
_cpct_memset_f8_asm::
   di                            ;; [ 4] Disable interrupts first
   ld   (msf_restoreSP + 1), sp  ;; [20]
   jp   msf8_asm_entry           ;; [10]

;; C call entry point
_cpct_memset_f8::
   di                            ;; [ 4] Disable interrupts first
   ld   (msf_restoreSP + 1), sp  ;; [20]
   ;; Recover parameters from stack
   pop  af           ;; [10] AF = Return address
   pop  hl           ;; [10] HL = Array pointer
   pop  bc           ;; [10] BC = Size of the array
   pop  de           ;; [10] DE = value to be set
                     ;; No need to restore them, as sp will be directly restored later on

msf8_asm_entry:
   ;; Move SP to the end of the array
   add  hl, bc       ;; [11] HL += BC (HL points to the end of the array)
   ld   sp, hl       ;; [ 6] SP = HL  (SP points to the end of the array)

   ;; Calculate the total number of chunks to copy
   srl b             ;; [ 8] BC = BC / 8 (using 3 right shifts)
   rr c              ;; [ 8]
   srl b             ;; [ 8]
   rr c              ;; [ 8]
   srl b             ;; [ 8]
   rr c              ;; [ 8]

msf_copyloop:
   push de               ;; [11] Push a chunck of 8-bytes to memory, 2-by-2
   push de               ;; [11]
   push de               ;; [11]
   push de               ;; [11]
   dec  bc               ;; [ 6] 1 chunck less
   jp   nz, msf_copyloop ;; [10] Continue if there still are more chuncks (BC != 0)

msf_restoreSP:
   ld   sp, #0000    ;; [10] Placeholder for restoring SP value before returning
   ei                ;; [ 4] Reenable interrupts

   ret               ;; [10] Return  
