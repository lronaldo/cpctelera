;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 Augusto Ruiz / RetroWorks (@Augurui)
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
.module cpct_sprites

;;
;; C bindings for <cpct_drawTileGrayCode2x8_af>
;;
;;   12 us, 3 bytes
;;
_cpct_drawTileGrayCode2x8_af::

   ;; GET Parameters from the stack
   pop  hl     ;; [3] HL = Return Address
   pop  de     ;; [3] DE = Pointer to video memory location where the sprite will be drawn
   ex (sp), hl ;; [6] HL = Pointer to the end of the sprite array...
               ;;     ...  and put returning address in the stack again
               ;;     ... as this function uses __z88dk_callee convention

.include /cpct_drawTileGrayCode2x8_af.asm/