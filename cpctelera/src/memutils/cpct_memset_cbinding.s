;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_memutils

.include /memutils.s/

;;
;; C call binding for <cpct_memset>
;;
_cpct_memset::
   ;; Recover parameters from stack
   ld   hl, #2       ;; [3] Make HL point to the byte where parameters start in the
   add  hl, sp       ;; [3] ... stack (first 2 bytes are return address)
   ld    e, (hl)     ;; [2] DE = Pointer to first byte in memory for memset
   inc  hl           ;; [2]
   ld    d, (hl)     ;; [2] 
   inc  hl           ;; [2]
   ldi               ;; [5] (HL)->(DE) Copy value to the first byte of the memory to be set
                     ;; .... and, at the same time, do INC HL, INC DE and DEC BC
   ld    c, (hl)     ;; [2] BC = Amount of bytes in memory to set to the value of A
   inc  hl           ;; [2]
   ld    b, (hl)     ;; [2]

.include /cpct_memset.asm/
