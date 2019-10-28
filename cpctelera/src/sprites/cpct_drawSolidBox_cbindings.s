;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2019 Arnaud Bouche (@Arnaud6128)
;;  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_sprites

;; Macros for easy use of undocumented opcodes
.include "macros/cpct_undocumentedOpcodes.h.s"

;;
;; C bindings for <cpct_drawSolidBox>
;;
;;   19 us, 7 bytes
;;
_cpct_drawSolidBox::
   ;; GET Parameters from the stack 
   pop   af          ;; [3] AF = Return address
   pop   de          ;; [3] DE = Video Memory Address
   pop   hl          ;; [3] L = Colour Pattern
   dec   sp          ;; [2] SP-- (To get next 2 bytes aligned with Memory Address)
   pop   bc          ;; [3] B = Height, C = Width
   push  af          ;; [4] Leave return address in the stack to fullfill __z88dk_callee convention
   ld     a, l       ;; [1] A = Colour Pattern

.include /cpct_drawSolidBox.asm/
