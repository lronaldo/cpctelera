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
.include "macros/cpct_reverseBits.h.s"
.include "macros/cpct_maths.h.s"

;;
;; C bindings for <cpct_drawSpriteHFlipM2>
;;
;;   28 us, 10 bytes
;;

_cpct_drawSpriteHFlipM2::
;; Parameter retrieval
   pop   af    ;; [3] AF = Return address
   pop   de    ;; [3] DE = Sprite start address pointer
   pop   hl    ;; [3] HL = Video memory address to draw the sprite
   pop   bc    ;; [3] BC = height / width
   push  af    ;; [4] Leave only return address in the stack, 
               ;; ... as this function uses __z88dk_callee convention
   
   push  ix    ;; [5] Save IX

.include /cpct_drawSpriteHFlipM2.asm/

   pop   ix    ;; [4] Restore IX before returning
   ret         ;; [3] Return to caller