;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2019 Arnaud bouche (@Arnaud6128)
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
   pop   af         ;; [3] AF = Return Address
   pop   hl         ;; [3] HL = First Parameter (Video memory pointer)
   dec   sp         ;; [2] Move SP 1 byte as next parameter (Colour Pattern is 1-Byte length)
   pop   de         ;; [3] DE = 1-Byte Colour Pattern (D = 1-Byte Colour Pattern, E = not used)
   ld    e, d       ;; [1] E = D (Colour Pattern)
   pop   bc         ;; [3] BC = Height/Width (B = Height, C = Width)
   push  af         ;; [4] Put returning address in the stack again as this function uses __z88dk_callee convention

.include /cpct_drawSolidBox.asm/