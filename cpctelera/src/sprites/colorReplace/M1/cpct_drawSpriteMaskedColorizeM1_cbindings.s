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
;;   33 us, 13 bytes
;;
_cpct_drawSpriteMaskedColorizeM1::

   ;; GET Parameters from the stack 
   ld (dms_restore_ix + 2), ix  ;; [6] Save IX to restore it before returning
   pop   hl                     ;; [3] HL = Return Address
   
   exx 
   pop   hl                     ;; [3] HL' = Source address (Sprite)
   pop   de                     ;; [3] DE' = Destination Memory
   pop   bc                     ;; [5] BC' = (B = Sprite Height, C = Width)
   exx
   
   ex   (sp), hl                ;; [6] HL = (H = newColor, L = oldColor)
                                ;; ... and leave Return Address at (SP) as we don't need to restore
                                ;; ... stack status because callin convention is __z88dk_callee

.include /cpct_drawSpriteMaskedColorizeM1.asm/

dms_restore_ix:
   ld   ix, #0000               ;; [4] Restore IX before returning
   ret                          ;; [3] Return to caller