;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; File: Undocumented Opcodes
;;
;;    Macros to clarify source code when using undocumented opcodes. Only
;; valid to be used from assembly language (not from C).
;;

;; Macro: jr__0
;;    Opcode for "JR #0" instruction
;; 
.macro jr__0
   .DW #0x0018  ;; JR #00 (Normally used as a modifiable jump, as jr 0 is an infinite loop)
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;; SLL Instructions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,

;; Macro: sll__b
;;    Opcode for "SLL b" instruction
;; 
.macro sll__b
   .db #0xCB, #0x30  ;; Opcode for sll b
.endm

;; Macro: sll__c
;;    Opcode for "SLL c" instruction
;; 
.macro sll__c
   .db #0xCB, #0x31  ;; Opcode for sll c
.endm

;; Macro: sll__d
;;    Opcode for "SLL d" instruction
;; 
.macro sll__d
   .db #0xCB, #0x32  ;; Opcode for sll d
.endm

;; Macro: sll__e
;;    Opcode for "SLL e" instruction
;; 
.macro sll__e
   .db #0xCB, #0x33  ;; Opcode for sll e
.endm

;; Macro: sll__h
;;    Opcode for "SLL h" instruction
;; 
.macro sll__h
   .db #0xCB, #0x34  ;; Opcode for sll h
.endm

;; Macro: sll__l
;;    Opcode for "SLL l" instruction
;; 
.macro sll__l
   .db #0xCB, #0x35  ;; Opcode for sll l
.endm

;; Macro: sll___hl_
;;    Opcode for "SLL (hl)" instruction
;; 
.macro sll___hl_
   .db #0xCB, #0x36  ;; Opcode for sll (hl)
.endm

;; Macro: sll__a
;;    Opcode for "SLL a" instruction
;; 
.macro sll__a
   .db #0xCB, #0x37  ;; Opcode for sll a
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;; IXL Related Macros
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,

;; Macro: ld__ixl    Value
;;    Opcode for "LD ixl, Value" instruction
;;  
;; Parameters:
;;    Value - An inmediate 8-bits value that will be loaded into ixl
;; 
.macro ld__ixl    Value 
   .db #0xDD, #0x2E, Value  ;; Opcode for ld ixl, Value
.endm

;; Macro: ld__ixl_a
;;    Opcode for "LD ixl, a" instruction
;; 
.macro ld__ixl_a
   .dw #0x6FDD  ;; Opcode for ld ixl, a
.endm

;; Macro: ld__ixl_b
;;    Opcode for "LD ixl, B" instruction
;; 
.macro ld__ixl_b
   .dw #0x68DD  ;; Opcode for ld ixl, b
.endm

;; Macro: ld__ixl_c
;;    Opcode for "LD ixl, C" instruction
;; 
.macro ld__ixl_c
   .dw #0x69DD  ;; Opcode for ld ixl, c
.endm

;; Macro: ld__ixl_d
;;    Opcode for "LD ixl, D" instruction
;; 
.macro ld__ixl_d
   .dw #0x6ADD  ;; Opcode for ld ixl, d
.endm

;; Macro: ld__ixl_e
;;    Opcode for "LD ixl, E" instruction
;; 
.macro ld__ixl_e
   .dw #0x6BDD  ;; Opcode for ld ixl, e
.endm

;; Macro: ld__ixl_ixh
;;    Opcode for "LD ixl, IXH" instruction
;; 
.macro ld__ixl_ixh
   .dw #0x6CDD  ;; Opcode for ld ixl, ixh
.endm

;; Macro: ld__a_ixl
;;    Opcode for "LD A, ixl" instruction
;; 
.macro ld__a_ixl
   .dw #0x7DDD  ;; Opcode for ld a, ixl
.endm

;; Macro: ld__b_ixl
;;    Opcode for "LD B, ixl" instruction
;; 
.macro ld__b_ixl
   .dw #0x45DD  ;; Opcode for ld b, ixl
.endm

;; Macro: ld__c_ixl
;;    Opcode for "LD c, ixl" instruction
;; 
.macro ld__c_ixl
   .dw #0x4DDD  ;; Opcode for ld c, ixl
.endm

;; Macro: ld__d_ixl
;;    Opcode for "LD D, ixl" instruction
;; 
.macro ld__d_ixl
   .dw #0x55DD  ;; Opcode for ld d, ixl
.endm

;; Macro: ld__e_ixl
;;    Opcode for "LD e, ixl" instruction
;; 
.macro ld__e_ixl
   .dw #0x5DDD  ;; Opcode for ld e, ixl
.endm

;; Macro: add__ixl
;;    Opcode for "Add ixl" instruction
;; 
.macro add__ixl
   .dw #0x85DD  ;; Opcode for add ixl
.endm

;; Macro: sub__ixl
;;    Opcode for "SUB ixl" instruction
;; 
.macro sub__ixl
   .dw #0x95DD  ;; Opcode for sub ixl
.endm

;; Macro: adc__ixl
;;    Opcode for "ADC ixl" instruction
;; 
.macro adc__ixl
   .dw #0x8DDD  ;; Opcode for adc ixl
.endm

;; Macro: sbc__ixl
;;    Opcode for "SBC ixl" instruction
;; 
.macro sbc__ixl
   .dw #0x9DDD  ;; Opcode for sbc ixl
.endm

;; Macro: and__ixl
;;    Opcode for "AND ixl" instruction
;; 
.macro and__ixl
   .dw #0xA5DD  ;; Opcode for and ixl
.endm

;; Macro: or__ixl
;;    Opcode for "OR ixl" instruction
;; 
.macro or__ixl
   .dw #0xB5DD  ;; Opcode for or ixl
.endm

;; Macro: xor__ixl
;;    Opcode for "XOR ixl" instruction
;; 
.macro xor__ixl
   .dw #0xADDD  ;; Opcode for xor ixl
.endm

;; Macro: cp__ixl
;;    Opcode for "CP ixl" instruction
;; 
.macro cp__ixl
   .dw #0xBDDD  ;; Opcode for cp ixl
.endm

;; Macro: dec__ixl
;;    Opcode for "DEC ixl" instruction
;; 
.macro dec__ixl
   .dw #0x2DDD  ;; Opcode for dec ixl
.endm

;; Macro: inc__ixl
;;    Opcode for "INC ixl" instruction
;; 
.macro inc__ixl
   .dw #0x2CDD  ;; Opcode for inc ixl
.endm


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;; IXH Related Macros
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,

;; Macro: ld__ixh    Value
;;    Opcode for "LD IXH, Value" instruction
;;  
;; Parameters:
;;    Value - An inmediate 8-bits value that will be loaded into IXH
;; 
.macro ld__ixh    Value 
   .db #0xDD, #0x26, Value  ;; Opcode for ld ixh, Value
.endm

;; Macro: ld__ixh_a
;;    Opcode for "LD IXH, a" instruction
;; 
.macro ld__ixh_a
   .dw #0x67DD  ;; Opcode for ld ixh, a
.endm

;; Macro: ld__ixh_b
;;    Opcode for "LD IXH, B" instruction
;; 
.macro ld__ixh_b
   .dw #0x60DD  ;; Opcode for ld ixh, b
.endm

;; Macro: ld__ixh_c
;;    Opcode for "LD IXH, C" instruction
;; 
.macro ld__ixh_c
   .dw #0x61DD  ;; Opcode for ld ixh, c
.endm

;; Macro: ld__ixh_d
;;    Opcode for "LD IXH, D" instruction
;; 
.macro ld__ixh_d
   .dw #0x62DD  ;; Opcode for ld ixh, d
.endm

;; Macro: ld__ixh_e
;;    Opcode for "LD IXH, E" instruction
;; 
.macro ld__ixh_e
   .dw #0x63DD  ;; Opcode for ld ixh, e
.endm

;; Macro: ld__ixh_ixl
;;    Opcode for "LD IXH, IXL" instruction
;; 
.macro ld__ixh_ixl
   .dw #0x65DD  ;; Opcode for ld ixh, ixl
.endm

;; Macro: ld__a_ixh
;;    Opcode for "LD A, IXH" instruction
;; 
.macro ld__a_ixh
   .dw #0x7CDD  ;; Opcode for ld a, ixh
.endm

;; Macro: ld__b_ixh
;;    Opcode for "LD B, IXH" instruction
;; 
.macro ld__b_ixh
   .dw #0x44DD  ;; Opcode for ld b, ixh
.endm

;; Macro: ld__c_ixh
;;    Opcode for "LD c, IXH" instruction
;; 
.macro ld__c_ixh
   .dw #0x4CDD  ;; Opcode for ld c, ixh
.endm

;; Macro: ld__d_ixh
;;    Opcode for "LD D, IXH" instruction
;; 
.macro ld__d_ixh
   .dw #0x54DD  ;; Opcode for ld d, ixh
.endm

;; Macro: ld__e_ixh
;;    Opcode for "LD e, IXH" instruction
;; 
.macro ld__e_ixh
   .dw #0x5CDD  ;; Opcode for ld e, ixh
.endm

;; Macro: add__ixh
;;    Opcode for "ADD IXH" instruction
;; 
.macro add__ixh
   .dw #0x84DD  ;; Opcode for add ixh
.endm

;; Macro: sub__ixh
;;    Opcode for "SUB IXH" instruction
;; 
.macro sub__ixh
   .dw #0x94DD  ;; Opcode for sub ixh
.endm

;; Macro: adc__ixh
;;    Opcode for "ADC IXH" instruction
;; 
.macro adc__ixh
   .dw #0x8CDD  ;; Opcode for adc ixh
.endm

;; Macro: sbc__ixh
;;    Opcode for "SBC IXH" instruction
;; 
.macro sbc__ixh
   .dw #0x9CDD  ;; Opcode for sbc ixh
.endm

;; Macro: and__ixh
;;    Opcode for "AND IXH" instruction
;; 
.macro and__ixh
   .dw #0xA4DD  ;; Opcode for and ixh
.endm

;; Macro: or__ixh
;;    Opcode for "OR IXH" instruction
;; 
.macro or__ixh
   .dw #0xB4DD  ;; Opcode for or ixh
.endm

;; Macro: xor__ixh
;;    Opcode for "XOR IXH" instruction
;; 
.macro xor__ixh
   .dw #0xACDD  ;; Opcode for xor ixh
.endm

;; Macro: cp__ixh
;;    Opcode for "CP IXH" instruction
;; 
.macro cp__ixh
   .dw #0xBCDD  ;; Opcode for cp ixh
.endm

;; Macro: dec__ixh
;;    Opcode for "DEC IXH" instruction
;; 
.macro dec__ixh
   .dw #0x25DD  ;; Opcode for dec ixh
.endm

;; Macro: inc__ixh
;;    Opcode for "INC IXH" instruction
;; 
.macro inc__ixh
   .dw #0x24DD  ;; Opcode for inc ixh
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;; IYL Related Macros
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,

;; Macro: ld__iyl    Value
;;    Opcode for "LD iyl, Value" instruction
;;  
;; Parameters:
;;    Value - An inmediate 8-bits value that will be loaded into iyl
;; 
.macro ld__iyl    Value 
   .db #0xFD, #0x2E, Value  ;; Opcode for ld iyl, Value
.endm

;; Macro: ld__iyl_a
;;    Opcode for "LD iyl, a" instruction
;; 
.macro ld__iyl_a
   .dw #0x6FFD  ;; Opcode for ld iyl, a
.endm

;; Macro: ld__iyl_b
;;    Opcode for "LD iyl, B" instruction
;; 
.macro ld__iyl_b
   .dw #0x68FD  ;; Opcode for ld iyl, b
.endm

;; Macro: ld__iyl_c
;;    Opcode for "LD iyl, C" instruction
;; 
.macro ld__iyl_c
   .dw #0x69FD  ;; Opcode for ld iyl, c
.endm

;; Macro: ld__iyl_d
;;    Opcode for "LD iyl, D" instruction
;; 
.macro ld__iyl_d
   .dw #0x6AFD  ;; Opcode for ld iyl, d
.endm

;; Macro: ld__iyl_e
;;    Opcode for "LD iyl, E" instruction
;; 
.macro ld__iyl_e
   .dw #0x6BFD  ;; Opcode for ld iyl, e
.endm

;; Macro: ld__iyl_iyh
;;    Opcode for "LD iyl, IXL" instruction
;; 
.macro ld__iyl_iyh
   .dw #0x6CFD  ;; Opcode for ld iyl, ixl
.endm

;; Macro: ld__a_iyl
;;    Opcode for "LD A, iyl" instruction
;; 
.macro ld__a_iyl
   .dw #0x7DFD  ;; Opcode for ld a, iyl
.endm

;; Macro: ld__b_iyl
;;    Opcode for "LD B, iyl" instruction
;; 
.macro ld__b_iyl
   .dw #0x45FD  ;; Opcode for ld b, iyl
.endm

;; Macro: ld__c_iyl
;;    Opcode for "LD c, iyl" instruction
;; 
.macro ld__c_iyl
   .dw #0x4DFD  ;; Opcode for ld c, iyl
.endm

;; Macro: ld__d_iyl
;;    Opcode for "LD D, iyl" instruction
;; 
.macro ld__d_iyl
   .dw #0x55FD  ;; Opcode for ld d, iyl
.endm

;; Macro: ld__e_iyl
;;    Opcode for "LD e, iyl" instruction
;; 
.macro ld__e_iyl
   .dw #0x5DFD  ;; Opcode for ld e, iyl
.endm

;; Macro: add__iyl
;;    Opcode for "Add iyl" instruction
;; 
.macro add__iyl
   .dw #0x85FD  ;; Opcode for add iyl
.endm

;; Macro: sub__iyl
;;    Opcode for "SUB iyl" instruction
;; 
.macro sub__iyl
   .dw #0x95FD  ;; Opcode for sub iyl
.endm

;; Macro: adc__iyl
;;    Opcode for "ADC iyl" instruction
;; 
.macro adc__iyl
   .dw #0x8DFD  ;; Opcode for adc iyl
.endm

;; Macro: sbc__iyl
;;    Opcode for "SBC iyl" instruction
;; 
.macro sbc__iyl
   .dw #0x9DFD  ;; Opcode for sbc iyl
.endm

;; Macro: and__iyl
;;    Opcode for "AND iyl" instruction
;; 
.macro and__iyl
   .dw #0xA5FD  ;; Opcode for and iyl
.endm

;; Macro: or__iyl
;;    Opcode for "OR iyl" instruction
;; 
.macro or__iyl
   .dw #0xB5FD  ;; Opcode for or iyl
.endm

;; Macro: xor__iyl
;;    Opcode for "XOR iyl" instruction
;; 
.macro xor__iyl
   .dw #0xADFD  ;; Opcode for xor iyl
.endm

;; Macro: cp__iyl
;;    Opcode for "CP iyl" instruction
;; 
.macro cp__iyl
   .dw #0xBDFD  ;; Opcode for cp iyl
.endm

;; Macro: dec__iyl
;;    Opcode for "DEC iyl" instruction
;; 
.macro dec__iyl
   .dw #0x2DFD  ;; Opcode for dec iyl
.endm

;; Macro: inc__iyl
;;    Opcode for "INC iyl" instruction
;; 
.macro inc__iyl
   .dw #0x2CFD  ;; Opcode for inc iyl
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;; IYH Related Macros
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,

;; Macro: ld__iyh    Value
;;    Opcode for "LD iyh, Value" instruction
;;  
;; Parameters:
;;    Value - An inmediate 8-bits value that will be loaded into iyh
;; 
.macro ld__iyh    Value 
   .db #0xFD, #0x26, Value  ;; Opcode for ld iyh, Value
.endm

;; Macro: ld__iyh_a
;;    Opcode for "LD iyh, a" instruction
;; 
.macro ld__iyh_a
   .dw #0x67FD  ;; Opcode for ld iyh, a
.endm

;; Macro: ld__iyh_b
;;    Opcode for "LD iyh, B" instruction
;; 
.macro ld__iyh_b
   .dw #0x60FD  ;; Opcode for ld iyh, b
.endm

;; Macro: ld__iyh_c
;;    Opcode for "LD iyh, C" instruction
;; 
.macro ld__iyh_c
   .dw #0x61FD  ;; Opcode for ld iyh, c
.endm

;; Macro: ld__iyh_d
;;    Opcode for "LD iyh, D" instruction
;; 
.macro ld__iyh_d
   .dw #0x62FD  ;; Opcode for ld iyh, d
.endm

;; Macro: ld__iyh_e
;;    Opcode for "LD iyh, E" instruction
;; 
.macro ld__iyh_e
   .dw #0x63FD  ;; Opcode for ld iyh, e
.endm

;; Macro: ld__iyh_iyl
;;    Opcode for "LD iyh, IyL" instruction
;; 
.macro ld__iyh_iyl
   .dw #0x65FD  ;; Opcode for ld iyh, iyl
.endm

;; Macro: ld__a_iyh
;;    Opcode for "LD A, iyh" instruction
;; 
.macro ld__a_iyh
   .dw #0x7CFD  ;; Opcode for ld a, iyh
.endm

;; Macro: ld__b_iyh
;;    Opcode for "LD B, iyh" instruction
;; 
.macro ld__b_iyh
   .dw #0x44FD  ;; Opcode for ld b, iyh
.endm

;; Macro: ld__c_iyh
;;    Opcode for "LD c, iyh" instruction
;; 
.macro ld__c_iyh
   .dw #0x4CFD  ;; Opcode for ld c, iyh
.endm

;; Macro: ld__d_iyh
;;    Opcode for "LD D, iyh" instruction
;; 
.macro ld__d_iyh
   .dw #0x54FD  ;; Opcode for ld d, iyh
.endm

;; Macro: ld__e_iyh
;;    Opcode for "LD e, iyh" instruction
;; 
.macro ld__e_iyh
   .dw #0x5CFD  ;; Opcode for ld e, iyh
.endm

;; Macro: add__iyh
;;    Opcode for "Add iyh" instruction
;; 
.macro add__iyh
   .dw #0x84FD  ;; Opcode for add iyh
.endm

;; Macro: sub__iyh
;;    Opcode for "SUB iyh" instruction
;; 
.macro sub__iyh
   .dw #0x94FD  ;; Opcode for sub iyh
.endm

;; Macro: adc__iyh
;;    Opcode for "ADC iyh" instruction
;; 
.macro adc__iyh
   .dw #0x8CFD  ;; Opcode for adc iyh
.endm

;; Macro: sbc__iyh
;;    Opcode for "SBC iyh" instruction
;; 
.macro sbc__iyh
   .dw #0x9CFD  ;; Opcode for sbc iyh
.endm

;; Macro: and__iyh
;;    Opcode for "AND iyh" instruction
;; 
.macro and__iyh
   .dw #0xA4FD  ;; Opcode for and iyh
.endm

;; Macro: or__iyh
;;    Opcode for "OR iyh" instruction
;; 
.macro or__iyh
   .dw #0xB4FD  ;; Opcode for or iyh
.endm

;; Macro: xor__iyh
;;    Opcode for "XOR iyh" instruction
;; 
.macro xor__iyh
   .dw #0xACFD  ;; Opcode for xor iyh
.endm

;; Macro: cp__iyh
;;    Opcode for "CP iyh" instruction
;; 
.macro cp__iyh
   .dw #0xBCFD  ;; Opcode for cp iyh
.endm

;; Macro: dec__iyh
;;    Opcode for "DEC iyh" instruction
;; 
.macro dec__iyh
   .dw #0x25FD  ;; Opcode for dec iyh
.endm

;; Macro: inc__iyh
;;    Opcode for "INC iyh" instruction
;; 
.macro inc__iyh
   .dw #0x24FD  ;; Opcode for inc iyh
.endm