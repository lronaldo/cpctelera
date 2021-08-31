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

.include "macros/cpct_undocumentedOpcodes.h.s"

;;
;; C bindings for <cpct_spriteColourizeM1>
;;
;;   15 us, 4 bytes
;;
_cpct_spriteColourizeM1::
   ;; GET Parameters from the stack 
   pop   hl                      ;; [3] HL = Return Address  
   pop   de                      ;; [3] DE = Replace Pattern (D=Find Pattern [OldPen], E=Insert Pattern (NewPen))
   pop   bc                      ;; [3] BC = Size of the array/sprite (width*height)
   ex   (sp), hl                 ;; [6] HL = Pointer to the sprite
                                 ;; ... and leave Return Address at (SP) as we don't need to restore
                                 ;; ... stack status because callin convention is __z88dk_callee
   
   ;; Include Common code
   .include /cpct_spriteColourizeM1.asm/
   
   ;; Generate the code with just 1 increment of HL at the end of every loop pass
   ;; as the array/sprite is to be composed of consecutive bytes 
   cpctm_generate_spriteColourizeM1 1

