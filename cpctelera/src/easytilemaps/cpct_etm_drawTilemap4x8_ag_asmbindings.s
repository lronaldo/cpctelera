;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_easytilemaps
.include "macros/cpct_undocumentedOpcodes.h.s"

;;
;; ASM bindings for <cpct_etm_drawTilemap4x8_ag_asm>
;;
;; 3 microseconds, 1 byte
;;
cpct_etm_drawTilemap4x8_ag_asm::

.include /cpct_etm_drawTilemap4x8_ag.asm/

   drawTilemap4x8_ag_gen cpct_etm_dtm4x8_ag_asm_

   ret               ;; [3] Return
