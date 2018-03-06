;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
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
.module cpct_memutils

;;
;; C Binding for cpct_memset_f64_i
;;
;;   16 us, 5 bytes
;;
_cpct_memset_f64_i::
   ;; Recover parameters from stack
   pop  af   ;; [3] AF = Return address
   pop  hl   ;; [3] HL = Array pointer
   pop  de   ;; [3] DE = value to be set
   pop  bc   ;; [3] BC = Size of the array

   push af   ;; [4] Put returning address in the stack again
             ;;      as this function uses __z88dk_callee convention


.include /cpct_memset_f64_i.asm/   ;; Include function code