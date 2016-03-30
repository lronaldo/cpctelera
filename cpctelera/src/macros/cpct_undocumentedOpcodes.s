;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;
;; Macros to clarify source code when using undocumented opcodes
;;

;; Macro: jr__0
;;    Opcode for "JR #0" instruction
;; 
.macro jr__0
   .DW #0x0018  ;; JR #00 (Normally used as a modifiable jump, as jr 0 is an infinite loop)
.endm

;; Macro: ld__ixl_c
;;    Opcode for "LD IXL, C" instruction
;; 
.macro ld__ixl_c
   .DW  #0x69DD  ;; ld ixl, c 
.endm

;; Macro: ld__c_ixl
;;    Opcode for "LD C, IXL" instruction
;; 
.macro ld__c_ixl
   .DW  #0x4DDD  ;; ld c, ixl
.endm

;; Macro: ld__ixl_b
;;    Opcode for "LD IXL, B" instruction
;; 
.macro ld__ixl_b
   .DW  #0x68DD    ;; ld ixl, b 
.endm

;; Macro: ld__b_ixl
;;    Opcode for "LD B, IXL" instruction
;; 
.macro ld__b_ixl
   .DW  #0x45DD    ;; ld b, ixl
.endm

;; Macro: ld__a_ixl
;;    Opcode for "LD A, IXL" instruction
;; 
.macro ld__a_ixl
   .dw #0x7DDD  ;; Opcode for ld a, ixl
.endm

;; Macro: ld__ixl_a
;;    Opcode for "LD IXL, A" instruction
;; 
.macro ld__ixl_a
   .dw #0x6FDD  ;; Opcode for ld ixl, a
.endm

;; Macro: dec__ixh
;;    Opcode for "DEC IXH" instruction
;; 
.macro dec__ixh
   .dw #0x25DD  ;; Opcode for dec ixh
.endm

;; Macro: dec__ixl
;;    Opcode for "DEC IXL" instruction
;; 
.macro dec__ixl
   .dw #0x2DDD  ;; Opcode for dec ixl
.endm

;; Macro: ld__ixl    Value
;;    Opcode for "LD IXL, Value" instruction
;;  
;; Parameters:
;;    Value - An inmediate 8-bits value that will be loaded into IXL
;; 
.macro ld__ixl    Value 
   .db #0xDD, #0x2E, Value  ;; Opcode for ld ixl, Value
.endm
