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
;; Function: cpct_setStackLocation
;;
;;    Sets a new location for the program stack in memory.
;;
;; C Definition:
;;    void <cpct_setStackLocation> (void* *memory*) __z88dk_fastcall;
;;
;; Input Parameters (2 Bytes):
;;  (2B HL) memory - Pointer to the new location in memory where stack will be placed.
;;
;; Assembly call (Input parameters on registers):
;;    call cpct_setStackLocation_asm
;;
;; Parameter Restrictions:
;;  * *memory* has no restiction. You may place stack whereever you wanted, but you
;; should be cautious: contents of the stack could erase data, code or essential 
;; system information. If that happens, your program may crash. Have special care 
;; with system rst vector (0x0000 - 0x003F) and firmware memory (if enabled).
;;
;; Important notes:
;;  * This function only changes program stack to a new location by modifying the
;; stack pointer (SP). It does not take care of present contents of the stack. If
;; you want to preserve present contents of the stack, copy them to the new location
;; (you may use <cpct_memcpy>).
;;
;; Details:
;;    Changes the location of the program stack by moving the stack pointer (SP) to 
;; a new *memory* location. Changing the program stack location is useful to have
;; greater control of it and to enable the 3rd memory bank (0x8000-0xBFFF) as double buffer 
;; zone, data storage or code memory.
;;
;; Destroyed Register values: 
;;    DE, HL, SP
;;
;; Required memory:
;;     bytes
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;   Any      |       7        |    28
;; -----------------------------------------
;;  * This function is identical called from C or from ASM
;; (end code)
;;
;; Credits:
;;    This routine was first sugested and developed by Lachlan Keown (@lachlank / CPCWiki)
;; in <this thread of CPCWiki at
;; http://www.cpcwiki.eu/forum/programming/cpctelera-1-0-amstrad-cpc-game-development-library-official-release/msg103937/>
;; This code is based on Lachlan's.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

_cpct_setStackLocation::
cpct_setStackLocation_asm::
   ;; Parameter *memory* is directly given in HL register, using __z88dk_fastcall convention

   pop  de      ;; [3] DE = Return address
   ld   sp, hl  ;; [2] SP = HL (SP points to new memory location for the program stack)
   ex   de, hl  ;; [1] HL = DE (HL holds Return address) 

   jp  (hl)     ;; [1] Return using retrieved stack address
