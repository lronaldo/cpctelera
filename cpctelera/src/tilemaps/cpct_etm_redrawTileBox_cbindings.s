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
;; C bindings for <cpct_etm_redrawTileBox>
;;
;;  23 microSecs, 6 bytes
;;
_cpct_etm_redrawTileBox::
  
   ld (restore_ix + 2), ix ;; [6] Save IX to restore it before returning
   pop  hl                 ;; [3] HL = Return address
   pop  ix                 ;; [5] IX = Pointer to <cpct_TEasyTilemap> structure
   pop  bc                 ;; [3] C = x coordinate, B = y coordinate
   ex   (sp), hl           ;; [6] H = height in tiles, L = width in tiles
                           ;; ... and leave Retrun Address at (SP) as we don't need to restore
                           ;; ... stack status because callin convention is __z88dk_callee

.include  /cpct_etm_redrawTileBox.asm/
