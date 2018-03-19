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
;; C bindings for <cpct_etm_drawTilemap4x8_agf>
;;
;; 33 microseconds, 12 bytes
;;
_cpct_etm_drawTilemap4x8_agf::
   ;; Parameters
   pop   hl          ;; [3] HL = Return address
   pop   de          ;; [3] DE = Video Memory Pointer
   ex  (sp), hl      ;; [6] HL = Tilemap Pointer, leaving previous HL value (return address)
                     ;; ... at the top of the stack (following __z88dk_callee convention)
   push  ix          ;; [5] Save IX and IY to let this function...
   push  iy          ;; [5] ...use and restore them before returning

.include /cpct_etm_drawTilemap4x8_agf.asm/

   drawTilemap4x8_agf_gen cpct_etm_dtm4x8_c_

   pop   iy          ;; [4] | Restore IX, IY
   pop   ix          ;; [4] |
   ret               ;; [3] Return
