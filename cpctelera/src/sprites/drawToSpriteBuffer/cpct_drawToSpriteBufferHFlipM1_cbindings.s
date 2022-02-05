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
.include "macros/cpct_reverseBits.h.s"
.include "macros/cpct_maths.h.s"
;;
;; C bindings for <cpct_drawToSpriteBufferHFlipM1>
;;
;;   31 us, 9 bytes
;;
_cpct_drawToSpriteBufferHFlipM1::
   ;; GET Parameters from the stack following __z88dk_callee convention
   pop  hl        ;; [3] HL  = Return Address
   pop  bc        ;; [3] C   = Back_Buffer_Width (B is ignored)
   pop  de        ;; [3] DE  = Pointer to Back Buffer destination 
   ld   a, c      ;; [1] A   = Back_Buffer_Width
   pop  bc        ;; [3] BC  = Height / Width (B = Height, C = Width)
   ex  (sp), hl   ;; [6] HL  = Pointer to Sprite <-> (SP) = Return Address : because z88dk_callee convention

   push ix        ;; [5] Save IX

.include /cpct_drawToSpriteBufferHFlipM1.asm/

   pop  ix        ;; [4] Restore IX before returning
   ret            ;; [3] Return to caller