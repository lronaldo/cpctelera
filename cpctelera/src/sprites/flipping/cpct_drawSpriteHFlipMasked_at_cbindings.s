;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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
.include "macros/cpct_maths.h.s"

;;
;; C bindings for <cpct_drawSpriteHFlipMasked_at>
;;
;;   34 us, 17 bytes
;;

_cpct_drawSpriteHFlipMasked_at::
   ld (restoreIX), ix   ;; [6] Save IX before using it

;; Parameter retrieval
   pop   af          ;; [3] AF = Return address
   pop   de          ;; [3] DE = Sprite start address pointer
   pop   hl          ;; [3] HL = Video memory address to draw the sprite
   pop   ix          ;; [4] IXL = Width, IXH = Height
   pop   bc          ;; [3] BC = Pointer to Pixel Flipping Table
   push  af          ;; [4] Leave only return address in the stack, 
                     ;; ... as this function uses __z88dk_callee convention
   ld    a, b        ;; [1] A = Most Significant byte of Pixel Flipping Table Address

.include /cpct_drawSpriteHFlipMasked_at.asm/

restoreIX = .+2
   ld    ix, #0000   ;; [4] Restore IX before returning
   ret               ;; [3] Return to caller