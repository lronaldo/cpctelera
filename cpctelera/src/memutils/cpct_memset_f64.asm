;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_memutils

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_memset_f64
;;
;;    Fills up a complete array in memory setting bytes 2-by-2, in chunks of 
;; 64 bytes. Size of the array must be multiple of 64.
;;
;; C Definition:
;;    void <cpct_memset_f64> (void* *array*, <u16> *value*, <u16> *size*);
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
;;  (2B DE) value - 16-bit value to be set (Pair of bytes)
;;  (2B BC) size  - Number of bytes to be set (>= 64, multiple of 64)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_memset_f64_asm
;;
;; Parameter Restrictions:
;;  * *array* could theoretically be any 16-bit memory location. However, take into 
;; account that this function does no check at all, and you could mistakenly overwrite 
;; important parts of your program, the screen, the firmware... Use it with care.
;;  * *size* must be greater than 63 and multiple of 64. It represents the size of the 
;; array, or the number of total bytes that will be set to the *value*. This function 
;; sets bytes 2-by-2, in chunks of 64 bytes, so the minimum amount of bytes to be set is 64. 
;; *Beware!* sizes below 64 can cause this function to *overwrite the entire memory*. 
;;  * *value* could be any 16-bit value, without restrictions. It is considered as 
;; a pair of bytes that will be copied to every 2-bytes in the array.
;;
;; Known limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    Sets all pairs of bytes of an *array* in memory to the same given *value*. This is 
;; the same operation as std memset from standard C library, but with the added advantage
;; of being faster and letting the user define the contents 16-bits-by-16-bits instead of
;; 8. The technique used by this function is as follows:
;;
;;  1 - It saves the value of SP to recover it at the end of the function
;;  2 - It places SP at the last 2-bytes of the array
;;  3 - It uses PUSH instructions to set bytes 2-by-2, in chunks of 64 bytes, until the entire array is set
;;
;;    This function works for array sizes from 64 to 65472. However, it is recommended 
;; that you use it for values much greater than 64. 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-binding   - 75 bytes
;;    ASM-binding - 70 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      |   microSecs (us)   |      CPU Cycles       |
;; ----------------------------------------------------------
;;    Any      | 50 + 132CH + 3CHHH | 200 + 528*CH + 12CHHH |
;; ----------------------------------------------------------
;;  CH%256 = 0 |         +1         |         +4            |
;; ----------------------------------------------------------
;; Asm saving  |        -16         |        -64            |
;; ----------------------------------------------------------
;; (end code)
;;    BC   = *array size* (Number of total bytes to set)
;;    CH   = BC \ 64 (number of *chunks*, 1 chunk = 64 bytes)
;;    CHHH = CH \ 256 - 1
;;     \ = integer division
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

   ;; The 16-bit value inside DE will be written to memory in little-endian form. 
   ;; If we want it to be in the order the user introduced it, D and E must be swapped
   ld   a, e         ;; [1] / 
   ld   e, d         ;; [1] | Swap D and E contents to simulate big-endian memory writing 
   ld   d, a         ;; [1] \ 

   ;; Save SP to restore it later, as this function makes use of it
   ld   (restore_sp), sp ;; [6] Save SP to recover it later on

   ;; Move SP to the end of the array
   add  hl, bc       ;; [3] HL += BC (HL points to the end of the array)
   di                ;; [1] Disable interrupts just before modifying SP
   ld   sp, hl       ;; [2] SP = HL  (SP points to the end of the array)

   ;; Calculate the total number of chunks to copy
   ld   a, c         ;; [1] BC = BC / 64 but with B and C interchanged
   rla               ;; [1]   - So this is the same than calculating BC /= 64 and then swap(B, C).
   rl   b            ;; [2]   - To do it, instead of 6 rotations to the right, we do 2 to the left,
   rla               ;; [1]     what gives as the results already swapped
   rl   b            ;; [2]   - We use A register instead of C (A=C) to save some cycles, as rla is
   rla               ;; [1]     50% faster than rl c and we can do the final AND operation directly
   and  #0x03        ;; [2] C = C % 4 (Only the last 2 bits of C are valid, as dividing by 64 should
   ld   c, a         ;; [1]            left us with the 6 most significant bits to 0)
      
   ;; B (contains NumberOfChunks % 256). That will be the number of chunks to copy on first pass.
   ;;    If B != 0, we copy C chunks to memory, then 256*(C-1) chunks to memory (Standard)
   ;;    IF B  = 0, we only have to copy 256*(C-1). That is, we discount first pass, as it is of C=0 chunks.
   dec  b            ;; [1]  Decrement B to test if it is 0 or not
   jp   p, startcopy ;; [3]  IF C = 0, then
   dec  c            ;; [1]    Discount first pass (C = 0 chunks), then continue doing B-1 passes of 256 chunks

startcopy:
   inc  b            ;; [1] Restore the actual value of b

copyloop:
   push de           ;; [4] Push a chunk of 64-bytes to memory, 2-by-2
   push de           ;; [4]  (So, 32 pushes)
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4] 
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   push de           ;; [4]
   djnz copyloop     ;; [4/3] 1 Less chunk. Continue if there still are more chunks (B != 0)
   dec  c            ;; [1] 256 less chunks (b runned up to 0, decrement c by 1)
   jp   p, copyloop  ;; [3] Continue 256 chunks more if C >= 0 (positive)

restore_sp = .+1
   ld   sp, #0000    ;; [3] Placeholder for restoring SP value before returning
   ei                ;; [1] Reenable interrupts

   ret               ;; [3] Return  
