;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; C bindings for <cpct_drawSpriteMasked>
;;
;;   28 us, 8 bytes
;;
_cpct_drawSpriteMasked::
   ;; GET Parameters from the stack 
   pop  af   ;; [3] AF = Return Address
   pop  hl   ;; [3] HL = Source Address (Sprite data array)
   pop  de   ;; [3] DE = Destination address (Video memory location)
   pop  bc   ;; [3] BC = Height/Width (B = Height, C = Width)
   push bc   ;; [4] Restore Stack status pushing values again
   push de   ;; [4] (Interrupt safe way, 6 cycles more)
   push hl   ;; [4]
   push af   ;; [4]

.include /cpct_drawSpriteMasked.asm/