;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
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
.module cpct_sprites

;;
;; C bindings for <cpct_spriteMaskedColourizeM0>
;;
;;   12 us, 3 bytes
;;
_cpct_spriteMaskedColourizeM0::
   ;; GET Parameters from the stack 
   pop   hl                     ;; [3] HL = Return Address  
   pop   bc                     ;; [3] B = Width, C = Height 
   ex   (sp), hl                ;; [6] HL = Pointer to the sprite
                                ;; ... and leave Return Address at (SP) as we don't need to restore
                                ;; ... stack status because callin convention is __z88dk_callee

.include /cpct_spriteMaskedColourizeM0.asm/
