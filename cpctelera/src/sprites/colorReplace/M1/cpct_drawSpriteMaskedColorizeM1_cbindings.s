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
;; C bindings for <cpct_drawSpriteMaskedColorizeM1>
;;
;;   33 us, 10 bytes
;;
_cpct_drawSpriteMaskedColorizeM1::
   ;; GET Parameters from the stack 
   pop   hl          ;; [3] HL = Return Address
   pop   af          ;; [3] AF = Source Sprite Pointer
   pop   de          ;; [3] DE = Destination video memory pointer
   pop   bc          ;; [3] BC = (B = Sprite Height, C = Width)
   ex   (sp), hl     ;; [6] HL = Replace Pattern (H=Find Pattern [OldPen], L=Insert Pattern (NewPen))
                     ;; ... and leave Return Address at (SP) as we don't need to restore
                     ;; ... stack status because callin convention is __z88dk_callee
					 
   push  ix          ;; [5] Save IX and IY to let this function...
   push  iy          ;; [5] ...use and restore them before returning        

.include /cpct_drawSpriteMaskedColorizeM1.asm/

   pop   iy          ;; [4] / Restore IX, IY
   pop   ix          ;; [4] \   
   ret               ;; [3] Return to caller