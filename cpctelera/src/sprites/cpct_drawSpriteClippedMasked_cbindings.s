;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.include "../macros/cpct_undocumentedOpcodes.s"

;;
;; C bindings for <cpct_drawSpriteClippedMasked>
;;
;;   28 us, 5 bytes
;;
_cpct_drawSpriteClippedMasked::
   ;; GET Parameters from the stack 
   pop  hl                      ;; [3] HL = Return address
   ld (simulated_return+1), hl  ;; [5] Save return address for simulated return
   
   dec  sp   ;; [2] Move SP 1 byte as next parameter (width_sprite_to_draw is 1-byte length)
   pop  af   ;; [3] A = Sprite width to Drawn

   pop  hl   ;; [3] HL = Source Address (Sprite data array)
   pop  de   ;; [3] DE = Destination address (Video memory location)
   pop  bc   ;; [3] BC = Height/Width (B = Height, C = Width)

.include /cpct_drawSpriteClippedMasked.asm/

simulated_return:
   ld   hl, #0000              ;; [3] HL = return address
   jp   (hl)                   ;; [1] Do a manual "ret"