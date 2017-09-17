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

;; Required macro definitions
.include "src/macros/cpct_reverseBits.s"

;;
;; C bindings for <cpct_hflipSpriteMaskedM1>
;;
;;   12 us, 3 bytes
;;
_cpct_hflipSpriteMaskedM1::
   ;; Parameter retrieval from stack
   pop  hl     ;; [3] HL = return address
   pop  bc     ;; [3] BC = height / width of the sprite
   ex (sp), hl ;; [6] HL = Sprite start address pointer, while leaving return address 
               ;; ... in the stack, as this function uses __z88dk_callee convention

.include /cpct_hflipSpriteMaskedM1.asm/
