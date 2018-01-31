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
;; AS; bindings for <cpct_drawSolidBox>
;;
;;   6 us, 4 bytes
;;
cpct_drawSolidBox_asm::    
   ld    h, d    ;; [1] HL = DE (HL Points to the first byte of the box, 
   ld    l, e    ;; [1] ... the one that will contain the colour pattern)
   ld (de), a    ;; [2] Copy colour pattern (first byte) to video memory
   inc  de       ;; [2] DE points to the next byte (where 2nd byte will be copied)

.include /cpct_drawSolidBox.asm/