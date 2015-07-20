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
;; Function: cpct_memcpy
;;
;;    Copies a bunch of bytes from one place in memory to other. 
;;
;; C Definition:
;;    void <cpct_memcpy> (void* *to*, const void* *from*, <u16> *size*) __z88dk_callee;
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
;;    C-bindings - 8 bytes
;;  ASM-bindings - 3 bytes
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;   Any      |    18 + 6S     |  72 + 24S
;; -----------------------------------------
;; Asm saving |      -16       |    -64
;; -----------------------------------------
;; (end code)
;;    S = *size* (Number of total bytes to set)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

   ldir              ;; [5/6] Copy all bytes, from source to destination

   ret               ;; [3] Return  
