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

;;
;; C bindings for <cpct_drawSpriteVFlipMasked>
;;
;;   31 us, 13 bytes
;;
_cpct_drawSpriteVFlipMasked::
   ;; GET Parameters from the stack 
   pop   af    ;; [3] AF = Return Address
   pop   de    ;; [3] DE = Source Address (Sprite data array)
   pop   hl    ;; [3] HL = Destination address (Video memory location)
   pop   bc    ;; [3] BC = Height/Width (B = Height, C = Width)
 
   push  af    ;; [4] Put returning address in the stack again
               ;;      as this function uses __z88dk_callee convention

   push  ix    ;; [5] Save IX into the stack
   ld     a, c ;; [1] A = width
   ld__ixh_b   ;; [2] IXH = height

.include /cpct_drawSpriteVFlipMasked.asm/

   pop   ix    ;; [4] Restore IX
   ret         ;; [3] Return to the caller