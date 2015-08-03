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
.module cpct_bitarray

;;
;; Array: cpct_bitWeights
;;
;;    Internal array that contains 8 byte-values, each one with 7 bits off and 1 bit on.
;; Each value is designed to be used as a mask for getting one bit from other byte using 
;; an AND operation. Hence, the name bitWeights. It is used by <cpct_getBit> and 
;; <cpct_setBit> functions. mainly.
;;
cpct_bitWeights:: .db #0x01, #0x02, #0x04, #0x08, #0x10, #0x20, #0x40, #0x80

;; This shuould be 8-byte aligned for better performance, but compiler is not able to do it
;; .bndry 8 ;; Make this vector start at a 8-byte aligned address to be able to use 8-bit arithmetic with pointers
;; bndry does not work when generated object file is linked later on.
