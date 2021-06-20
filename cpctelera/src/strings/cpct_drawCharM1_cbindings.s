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
.module cpct_strings

;;
;; Include constants and general values
;;
.include "strings.s"

;;
;; C bindings for <cpct_drawCharM1>
;;
;;   28 us, 13 bytes
;;
_cpct_drawCharM1::
   ;; GET Parameters from the stack 
   pop   hl          ;; [3] HL = Return Address
   pop   de          ;; [3] DE = Pointer to Video Memory
   ex    (sp), hl    ;; [6] L = ASCII Value of the character to be drawn, and
                     ;; ...leaving Return Address on top of the stack at the same time
                     ;; ...as this function uses __z88dk_callee convention
   ex    de, hl      ;; [1] E = ASCII Value, HL = Pointer to video memory
   ld   (saveix), ix ;; [6] Save IX value before using it

.include /cpct_drawCharM1.asm/

saveix = .+2
   ld    ix, #0000   ;; [6] Restore IX before returning (0000 is a placeholder)
   ret               ;; [3] Return
