;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;
;; File: LUTs (Look-Up-Tables)
;;
;;    Useful macros for accessing and managing Look-Up-Tables
;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_lutget8
;;
;;    Gets a value from a 256-byte-max 8-bit table into A register
;;
;; Parameters:
;;    Table         - Memory address where the 256-byte-max table starts. It can be 
;;  either an hexadecimal, decimal or octal address, or a symbol (the table name).
;;    TR1           - An 8-bits register from the set {B, D, H}
;;    TR2           - An 8-bits register from the set {C, E, L}. This register must
;;  match TR1 to form a valid 16-bits register (BC, DE or HL), as the register TR1'TR2
;;  will be loaded with the address of the table, to be the base pointer.
;; 
;; Input Registers: 
;;    A     - Index in the LUT to be accessed.
;;
;; Return Value:
;;    A     - Value got from the LUT ( table[TR1'TR2 + A] )
;;
;; Details:
;;    This macro gets a value from a table into the A register. The process is simple:
;;
;;    1. It loads the address of the table in the 16-bits register TR1'TR2
;;    2. It adds the index (A) to TR1'TR2  (TR1'TR2 += A)
;;    3. It loads into A register the byte pointed by TR1'TR2
;;
;; Modified Registers: 
;;    AF, TR1, TR2
;;
;; Required memory:
;;    9 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |      10       |     40
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.macro cpctm_lutget8 Table, TR1, TR2
    ld   TR1'TR2, #Table   ;; [3] TR1_TR2 points to the LUT
    
    ;; Compute TR1'TR2 += A
    add  TR2               ;; [1] | TR2 += A
    ld   TR2, a            ;; [1] |
    sub  a                 ;; [1] A = 0 (preserving Carry Flag)
    adc  TR1               ;; [1] | TR1 += Carry
    ld   TR1, a            ;; [1] |

    ;; A = *(TR1_TR2 + A)
    ld   a, (TR1'TR2)      ;; [2] A = Value stored at given index from the LUT 
.endm

