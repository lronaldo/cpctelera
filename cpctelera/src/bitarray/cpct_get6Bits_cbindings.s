;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_bitarray

;;
;; C-bindings for calling function <cpct_get6Bits>
;;
;;  12 microSecs, 3 bytes
;;
_cpct_get6Bits::
   ;; Recover parameters from the stack
   pop hl           ;; [3] HL = Return Address
   pop de           ;; [3] DE = Pointer to the array in memory
   ex (sp), hl      ;; [6] HL = Index of the nibble (6 bits) we want to get
                    ;; ... also putting again Return Address where SP is located now
                    ;; ... as this function is using __z88dk_callee convention


.include /cpct_get6Bits.asm/