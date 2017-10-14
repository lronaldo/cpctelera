;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 Bouche Arnaud
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.include "macros/cpct_maths.h.s" 

;;
;; ASM bindings for <cpct_drawToSpriteBufferMaskedAlignedTable>
;;
;;   3 us, 1 byte
;;
cpct_drawToSpriteBufferMaskedAlignedTable_asm::

   ;; User *should* preserve IX before calling this function, if required

.include /cpct_drawToSpriteBufferMaskedAlignedTable.asm/

   ;; Ret is included in C-bindings, so it must be here also
   ;; because the asm file does not include it
   ret             ;; [3] Return to caller
