;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_waitHalts
;;
;;    Waits for a given number of *halt* assembler instructions to be executed.
;;
;; C Definition:
;;    void <cpct_waitHalts> (<u8> n) __z88dk_fastcall;
;;
;; Input Parameters (1 Byte):
;;  (1B B) n - Number of halts to wait 
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_waitHalts_asm
;;
;; Parameter Restrictions:
;;  * *n* can be any 8-bits value. Take into account that a value of 0 will result
;; on 256 halts to be waited for, as it will be decremented before comparison.
;;
;; Known issues:
;;    * Warning: if interrupts are disabled when this function gets called, it 
;; will effectively hang your CPU forever, requiring a reset.
;;
;; Destroyed Register values: 
;;    F, B
;;
;; Required memory:
;;    C-bindings - 5 bytes
;;  ASM-bindings - 4 bytes
;;
;; Time Measures:
;;    Time taken by the function greatly depends on the moment it is called. The
;; function will effectively wait for *n* interrupts to be produced + 7 additional
;; microseconds for final DJNZ and RET instructions.
;;
;; Details:
;;    Executes a wait loop with a *halt* instruction inside. This effectively 
;; waits for *n* interrupt signals to be produced. Each *halt* instruction
;; hangs the CPU until the next interrupt signal is received. 
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   waitloop:
      halt           ;; [?] Make CPU hang until next interrupt is received
   djnz waitloop     ;; [3/4] B--. If B != 0, repeat the waitloop

   ret               ;; [3] Return  
