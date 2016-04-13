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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Variable: cpct_mxor32_seed
;;
;;    Contains the 32-bits seed for Marsaglia's XOR-shift random number generator.
;;
;; C Definition:
;;    <u32> <cpct_mxor32_seed>;
;;
;; Known limitations:
;;    * This seed should never be set to 0. Functions that use this seed will always
;; return 0 if this seed is set to 0.
;;    * You may assing a value to this seed direcly from C or ASM. If directly accessed 
;; from ASM, do not forget to put an underscore in front (_cpct_mxor32_seed).
;;
;; Used by:
;;    This seed variable is used by these functions,
;;    * <cpct_getRandom_mxor_u32>
;;    * <cpct_getRandom_mxor_u16>
;;    * <cpct_getRandom_mxor_u8>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_mxor32_seed:: .dw #0x1A7B, #0x59F2