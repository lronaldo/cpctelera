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
.module cpct_sprites

;; Macros for easy use of undocumented opcodes
.include "macros/cpct_undocumentedOpcodes.h.s"

;;
;; C bindings for <cpct_drawSpriteBlended>
;;
;;   15 us, 4 bytes
;;
_cpct_drawSpriteBlended::
   ;; GET Parameters from the stack 
   pop  hl     ;; [3] HL = Return Address
   pop  de     ;; [3] DE = Destination address (Video memory location)
   pop  bc     ;; [3] BC = Height/Width (B = Width, C = Height)
   ex (sp), hl ;; [6] HL = Source Address (Sprite data array)
               ;; ... And put returning address in the stack again
               ;;      as this function uses __z88dk_callee convention

.include /cpct_drawSpriteBlended.asm/