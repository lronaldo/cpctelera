;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; ASM bindings for <cpct_drawSpriteClipped>
;;
cpct_drawSpriteClipped_asm::         ;; Assembly entry point

   pop  hl                     ;; [3] HL = Return address
   ld (simulated_return+1), hl ;; [5] Save return address for simulated return

.include /cpct_drawSpriteClipped.asm/

simulated_return:
   ld   hl, #0000              ;; [3] HL = return address
   jp   (hl)                   ;; [1] Do a manual "ret"