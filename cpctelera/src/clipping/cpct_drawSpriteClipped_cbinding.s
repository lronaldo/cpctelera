  
;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2019 Arnaud Bouche (@Arnaud6128)
;;  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;void asm_drawSpriteClipped(u8 width_to_draw, void *sprite, u8 width, u8 height, void* memory) __z88dk_callee;
;;

;; C bindings for <cpct_drawSpriteClipped>
;;
;;   20 us, 6 bytes
;;
_cpct_drawSpriteClipped::
   ;; Get Parameters from the stack 
   pop  hl                       ;; [3] HL = Return Address
   dec  sp                       ;; [2] Move SP 1 byte as next parameter (width_to_draw is 1-byte length)
   pop  af                       ;; [3] A  = Sprite width_to_draw
   pop  de                       ;; [3] DE = Source pointer (Sprite data array)
   pop  bc                       ;; [3] BC = Height/Width (B = Height, C = Width) 
   ex  (sp), hl                  ;; [6] HL = Destination pointer (Video mem) <-> (SP) Returning back address because __z88dk_callee convention  
  
.include /cpct_drawSpriteClipped.asm/
