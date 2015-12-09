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
.module cpct_random

;;
;; C bindings for <cpct_setSeed_xsp40_u8>
;;
;;   16 us, 5 bytes
;;
_cpct_setSeed_xsp40_u8::
   pop  hl     ;; [3] HL = Return address
   pop  bc     ;; [3] BC: B ignored, C = 8-bits seed for Weyl sequence
   pop  de     ;; [3] DE = First  16bits from the 32bits seed
   ex (sp), hl ;; [6] HL = Second 16bits from the 32bits seed (and Return address left on top of the stack)
   ld    a, c  ;; [1] A = 8-bits seed for Weyl sequence

.include /cpct_setSeed_xsp40_u8.asm/
