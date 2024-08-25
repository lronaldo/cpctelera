;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_divideby10
  
;;
;; C bindings for <cpct_divideby10>
;;
;;   13 microSecs, 7 bytes
;;
_cpct_divideby10::

  ;; Get Parameters from stack
  pop  hl   ;; [3] AF = Return Address
  dec sp
  pop  af   ;; [3] AF = Destination address
  ;;pop  hl   ;; [3] HL = Source Address
  ;;pop  bc   ;; [3] BC = size - Number of bytes to be set (>= 1)

  push hl   ;; [4] Put returning address in the stack again 
               ;;      as this function uses __z88dk_callee convention
.include /cpct_divideby10.s/
;;Return L as result

ret