;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; File: Combined operations
;;
;;    Macros to clarify source code that combine several operations in one macro.
;; For instance, macros to copy HL to DE or IX to DE, that require 2 or more 
;; instructions but are commonly used.
;;
.include "macros/cpct_undocumentedOpcodes.h.s"

;; Macro: ld__hl_de
;;    Copy DE to HL, using 2 instructions
;; COST: 2 us (8 CPU Cycles)
;; 
.macro ld__hl_de
   ;; LD HL, DE
   ;;------------
   ld h, d
   ld l, e
   ;;------------
.endm

;; Macro: ld__de_hl
;;    Copy HL to DE, using 2 instructions (ld d, h : ld e, l)
;; COST: 2 us (8 CPU Cycles)
;; 
.macro ld__de_hl
   ;; LD DE, HL
   ;;------------
   ld d, h
   ld e, l
   ;;------------
.endm

;; Macro: ld__de_ix
;;    Copy IX to DE, using 2 instructions (ld e, ixl : ld d, ixh)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__de_ix
   ;; LD DE, IX
   ;;------------
   ld__e_ixl
   ld__d_ixh
   ;;------------
.endm

;; Macro: ld__bc_ix
;;    Copy IX to BC, using 2 instructions (ld c, ixl : ld b, ixh)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__bc_ix
   ;; LD BC, IX
   ;;------------
   ld__c_ixl
   ld__b_ixh
   ;;------------
.endm

;; Macro: ld__hl_ix
;;    Copy IX to HL, using 4 instructions. 
;;    Modifies A Register
;; COST: 6 us (24 CPU Cycles)
;; 
.macro ld__hl_ix
   ;; LD HL, IX
   ;;------------
   ld__a_ixl
   ld  l, a
   ld__a_ixh
   ld  h, a
   ;;------------
.endm

;; Macro: ld__ix_de
;;    Copy DE to IX, using 2 instructions (ld ixl, e : ld ixh, d)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__ix_de
   ;; LD IX, DE
   ;;------------
   ld__ixl_e
   ld__ixh_d
   ;;------------
.endm

;; Macro: ld__ix_bc
;;    Copy BX to IX, using 2 instructions (ld ixl, c : ld ixh, b)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__ix_bc
   ;; LD IX, BC
   ;;------------
   ld__ixl_c
   ld__ixh_b
   ;;------------
.endm

;; Macro: ld__ix_hl
;;    Copy HL to IX, using 4 instructions. 
;;    Modifies A Register
;; COST: 6 us (24 CPU Cycles)
;; 
.macro ld__ix_hl
   ;; LD IX, HL
   ;;------------
   ld  a, l
   ld__ixl_a
   ld  a, h
   ld__ixh_a
   ;;------------
.endm

;; Macro: ld__de_iy
;;    Copy IY to DE, using 2 instructions (ld e, iyl : ld d, iyh)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__de_iy
   ;; LD DE, IY
   ;;------------
   ld__e_iyl
   ld__d_iyh
   ;;------------
.endm

;; Macro: ld__bc_iy
;;    Copy IY to BC, using 2 instructions (ld c, iyl : ld b, iyh)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__bc_iy
   ;; LD BC, IY
   ;;------------
   ld__c_iyl
   ld__b_iyh
   ;;------------
.endm

;; Macro: ld__hl_iy
;;    Copy IY to HL, using 4 instructions. 
;;    Modifies A Register
;; COST: 6 us (24 CPU Cycles)
;; 
.macro ld__hl_iy
   ;; LD HL, IY
   ;;------------
   ld__a_iyl
   ld  l, a
   ld__a_iyh
   ld  h, a
   ;;------------
.endm

;; Macro: ld__iy_de
;;    Copy DE to IY, using 2 instructions (ld iyl, e : ld iyh, d)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__iy_de
   ;; LD IY, DE
   ;;------------
   ld__iyl_e
   ld__iyh_d
   ;;------------
.endm

;; Macro: ld__iy_bc
;;    Copy BX to IY, using 2 instructions (ld iyl, c : ld iyh, b)
;; COST: 4 us (16 CPU Cycles)
;; 
.macro ld__iy_bc
   ;; LD IY, BC
   ;;------------
   ld__iyl_c
   ld__iyh_b
   ;;------------
.endm

;; Macro: ld__iy_hl
;;    Copy HL to IY, using 4 instructions. 
;;    Modifies A Register
;; COST: 6 us (24 CPU Cycles)
;; 
.macro ld__iy_hl
   ;; LD IY, HL
   ;;------------
   ld  a, l
   ld__iyl_a
   ld  a, h
   ld__iyh_a
   ;;------------
.endm

;; Macro: ld__ix_iy
;;    Copy IY to IX, using 4 instructions. 
;;    Modifies A Register
;; Cost: 8 us (32 CPU Cycles)
;; 
.macro ld__ix_iy
   ;; LD IX, IY
   ;;------------
   ld__a_iyl
   ld__ixl_a
   ld__a_iyh
   ld__ixh_a
   ;;------------
.endm

;; Macro: ld__iy_ix
;;    Copy IX to IY, using 4 instructions. 
;;    Modifies A Register
;; Cost: 8 us (32 CPU Cycles)
;; 
.macro ld__iy_ix
   ;; LD IY, IX
   ;;------------
   ld__a_ixl
   ld__iyl_a
   ld__a_ixh
   ld__iyh_a
   ;;------------
.endm

;; Macro: ex__de_ix
;;    Swap DE with IX
;;    Modifies A Register
;; Cost: 10 us (40 CPU Cycles)
;; 
.macro ex__de_ix
   ;; EX DE, IX
   ;;------------
   ld a, e
   ld__e_ixl
   ld__ixl_a
   ld a, d
   ld__d_ixh
   ld__ixh_a
   ;;------------
.endm

;; Macro: ex__bc_ix
;;    Swap BC with IX
;;    Modifies A Register
;; Cost: 10 us (40 CPU Cycles)
;; 
.macro ex__bc_ix
   ;; EX BC, IX
   ;;------------
   ld a, c
   ld__c_ixl
   ld__ixl_a
   ld a, b
   ld__b_ixh
   ld__ixh_a
   ;;------------
.endm

;; Macro: ex__hl_ix
;;    Swap HL with IX
;;    Uses 2 bytes on the stack for the swap
;;    Modifies A register
;; Cost: 15 us (60 CPU Cycles)
;; 
.macro ex__hl_ix
   ;; EX HL, IX
   ;;------------
   push  hl
   ld__a_ixl
   ld l, a
   ld__a_ixh
   ld h, a
   pop   ix
   ;;------------
.endm

;; Macro: ex__de_iy
;;    Swap DE with IY
;;    Modifies A Register
;; Cost: 10 us (40 CPU Cycles)
;; 
.macro ex__de_iy
   ;; EX DE, IY
   ;;------------
   ld a, e
   ld__e_iyl
   ld__iyl_a
   ld a, d
   ld__d_iyh
   ld__iyh_a
   ;;------------
.endm

;; Macro: ex__bc_iy
;;    Swap BC with IY
;;    Modifies A Register
;; Cost: 10 us (40 CPU Cycles)
;; 
.macro ex__bc_iy
   ;; EX BC, IY
   ;;------------
   ld a, c
   ld__c_iyl
   ld__iyl_a
   ld a, b
   ld__b_iyh
   ld__iyh_a
   ;;------------
.endm

;; Macro: ex__hl_iy
;;    Swap HL with IY
;;    Uses 2 bytes on the stack for the swap
;;    Modifies A register
;; Cost: 15 us (60 CPU Cycles)
;; 
.macro ex__hl_iy
   ;; EX HL, IY
   ;;------------
   push  hl
   ld__a_iyl
   ld l, a
   ld__a_iyh
   ld h, a
   pop   iy
   ;;------------
.endm