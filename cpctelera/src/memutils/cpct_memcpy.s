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
;; Function: cpct_memcpy
;;
;;    Copies a bunch of bytes from one place in memory to other. 
;;
;; C Definition:
;;    void <cpct_memcpy> (void* *to*, const void* *from*, <u16> *size*);
;;
;; Input Parameters (6 Bytes):
;;  (2B DE) to   - Pointer to the destination (first byte where bytes will be written)
;;  (2B HL) from - Pointer to the source (first byte from which bytes will be read)
;;  (2B BC) size - Number of bytes to be set (>= 1)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_memcpy_asm
;;    * Note: you may prefer using Z80's *LDIR* instead.
;;
;; Parameter Restrictions:
;;  * *to*, *from* can be any 16-bit values. Just take into account that this 
;; function doest not do any kind of check, so you are responsible for where you
;; copy bytes to. If you accidentally copy bytes to an undesired location, unpredictable
;; things may happen.
;;  * *size* could also be any 16-bit value and, again, you are responsible for 
;; giving the right size *in bytes* that you wanted to copy. Again, giving an incorrect
;; value for size can make this routine to overwrite memory places you did not meant to,
;; causing unpredictable behaviour.
;;
;; Details:
;;    Copies a *size* bytes *from* one location *to* other in memory. This function
;; is an specific implementation of standard C memcpy for Z80 machines, and it is
;; fully compatible with the standard.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    11 bytes
;;
;; Time Measures:
;; (start code)
;;   Case     |   Cycles   | microSecs (us)
;; -----------------------------------------
;;   Any      | 89 + 21*S  | 22.25 + 5.25*S
;; -----------------------------------------
;; Asm saving |    -84     |   -21.00
;; -----------------------------------------
;; (end code)
;;    S = *size* (Number of total bytes to set)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

_cpct_memcpy::
   ;; Get parameters from stack
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   ld (mcp_restoreSP+1), sp   ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                         ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                    ;; [10] AF = Return Address
   pop  de                    ;; [10] DE = Destination address
   pop  hl                    ;; [10] HL = Source Address
   pop  bc                    ;; [10] BC = Array size
mcp_restoreSP:
   ld   sp, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   ei                         ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af           ;; [10] AF = Return Address
   pop  de           ;; [10] DE = Destination address
   pop  hl           ;; [10] HL = Source Address
   pop  bc           ;; [10] BC = Height/Width (B = Height, C = Width)
   push bc           ;; [11] Restore Stack status pushing values again
   push hl           ;; [11] (Interrupt safe way, 6 cycles more)
   push de           ;; [11]
   push af           ;; [11]
.endif

cpct_memcpy_asm::    ;; Assembly entry point
   ldir              ;; [21/16] Copy all bytes, from source to destination

   ret               ;; [10] Return  
