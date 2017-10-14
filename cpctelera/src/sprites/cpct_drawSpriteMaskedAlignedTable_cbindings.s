;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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
;; C bindings for <cpct_drawSpriteMaskedAlignedTable>
;;
;;    33 microSecs, 15 bytes
;;
_cpct_drawSpriteMaskedAlignedTable::

   ld (dms_restore_ix + 2), ix  ;; [6] Save IX to restore it before returning
   pop   hl       ;; [3] HL = Return Address
   pop   bc       ;; [3] BC = Pointer to the Sprite data
   pop   de       ;; [3] DE = Pointer to the place in video memory where sprite will be drawn
   pop   ix       ;; [5] IX = width (IXL) and height (IXH) of the sprite in bytes, 
   ex   (sp), hl  ;; [6] HL = Pointer to the mask table
                  ;; ... and leave Return Address at (SP) as we don't need to restore
                  ;; ... stack status because callin convention is __z88dk_callee

.include /cpct_drawSpriteMaskedAlignedTable.asm/

dms_restore_ix:
   ld   ix, #0000  ;; [4] Restore IX before returning
   ret             ;; [3] Return to caller
