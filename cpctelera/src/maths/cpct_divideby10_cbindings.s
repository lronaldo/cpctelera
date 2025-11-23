;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2024 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_maths

;;
;; C bindings for <cpct_divideby10>
;;
;;   5 microSecs, 3 bytes
;;
_cpct_divideby10::
   ld  a, l    ;; [1] A=L. Parameter is given in L register because of __z88dk_fastcall
               ;;	    calling convention, but must be placed in A register for working.

   .include /cpct_divideby10.asm/

   ld l, a     ;; [1] Put returning result in L register as this function
               ;;      uses __z88dk_fastcall convention
   ret         ;; [3]
