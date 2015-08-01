;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_tilemaps

;;
;; C bindings for <cpct_etm_drawTileMap2x4_f>
;;
;;  17 microSecs, 6 bytes
;;
_cpct_etm_drawTilemap2x4_f::
   ;; Recover parameters from the stack
   pop hl           ;; [3] HL = Return Address
   pop bc           ;; [3]  B = map_height, C = map_width
   pop de           ;; [3] DE = Pointer to video memory where to draw the tilemap

   ex (sp), hl      ;; [6] HL = Pointer to the start of the tilemap
                    ;; ... also putting again Return Address where SP is located now
                    ;; ... as this function is using __z88dk_callee convention
   ld   a, c        ;; [1] A = map_width
   ld   c, b        ;; [1] C = map_height

.include /cpct_etm_drawTilemap2x4_f.asm/
