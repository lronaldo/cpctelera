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
;;    Copy DE to HL, using 2 instructions (ld h, d : ld l, e)
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
;;    (ld a, ixl : ld l, a : ld a, ixh : ld h, a )
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
;;    ( ld a, l : ld ixl, a : ld a, h : ld ixh, a )
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
;;    (ld a, iyl : ld l, a : ld a, iyh : ld h, a )
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
;;    ( ld a, l : ld iyl, a : ld a, h : ld iyh, a )
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