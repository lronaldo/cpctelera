;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 César Nicolás González (CNGSoft)
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
.module cpct_loaders

;;
;; C bindings for <cpct_miniload>
;;
;;   28 us, 13 bytes
;;
_cpct_miniload::
   ld   (saveix), ix ;; [6] Save IX before using it

   pop   hl          ;; [3] HL = Return address
   pop   ix          ;; [5] IX = Loading memory address
   pop   de          ;; [3] DE = Size in bytes of the block to read from Cassette
   push  hl          ;; [4] Save return address on the top of the stack to fullfill
                     ;; .... __z88dk_callee convention

.include /cpct_miniload.asm/

saveix = .+2
   ld   ix, #0000    ;; [4] Restore IX (0000 is a placeholder)
   ret               ;; [3] Return to caller