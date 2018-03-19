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
;; C bindings for <cpct_etm_drawTilemap4x8_ag>
;;
;; 34 microseconds, 13 bytes
;;
_cpct_etm_drawTilemap4x8_ag::
   ;; Parameters
   pop   af          ;; [3] AF = Return address
   pop   hl          ;; [3] HL = Video Memory Pointer
   pop   de          ;; [3] DE = Tilemap Pointer
   push  af          ;; [4] Leave previous AF value (return address)
                     ;; ... at the top of the stack (following __z88dk_callee convention)
   push  ix          ;; [5] Save IX and IY to let this function...
   push  iy          ;; [5] ...use and restore them before returning

.include /cpct_etm_drawTilemap4x8_ag.asm/

   drawTilemap4x8_ag_gen cpct_etm_dtm4x8_ag_c_

   pop   iy          ;; [4] | Restore IX, IY
   pop   ix          ;; [4] |
   ret               ;; [3] Return
