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
.module cpct_easytilemaps

;;
;; ASM bindings for <cpct_etm_redrawTileBox>
;;
;;   12 microSecs, 8 bytes
;;
cpct_etm_drawTileBox2x4_asm::

   pop  hl                     ;; [3] HL = Return address
   ld (simulated_return+1), hl ;; [5] Save return address for simulated return

   ;; Leave pvideomem and ptilemap in the stack as they will be recovered later
   ;; during function operation

.include  /cpct_etm_drawTileBox2x4.asm/

simulated_return:
   ld   hl, #0000              ;; [3] HL = return address
   jp   (hl)                   ;; [1] Do a manual "ret"
