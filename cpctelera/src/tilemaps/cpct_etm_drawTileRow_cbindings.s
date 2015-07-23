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
;; C bindings for <cpct_etm_drawTileRow>
;;
;;    17 microSecs, 5 bytes
;;
_cpct_etm_drawTileRow::   
   ;; Recover parameters from the stack
   pop hl       ;; [3] HL = Return Address
   pop af       ;; [3]  A = Number of tiles in this row
   dec sp       ;; [2] 
   pop de       ;; [3] DE = Pointer to video memory where to draw the tiles

   ex (sp), hl  ;; [6] HL = Pointer to the start of the tilemap row
                ;; ... also puttin again Return Address where SP is located now
                ;; ... as this function is using __z88dk_callee convention