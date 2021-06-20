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
.module cpct_compression

;; Includes
.include "macros/cpct_undocumentedOpcodes.h.s"

;;
;; ASM bindings for <cpct_zx7b_decrunch>
;;
;;   12 microSecs, 3 bytes
;;
_cpct_zx7b_decrunch_s::

   pop   hl       ;; [3] Get Return address
   pop   de       ;; [3] DE = Destination
   ex   (sp), hl  ;; [6] HL = Source (leaving return address at the 
                  ;;     top of the stack, as convention is __z88dk_callee)

.include  /cpct_zx7b_decrunch_s.asm/
