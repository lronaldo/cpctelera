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


; MODULE: TileMaps
; This module helps in the managements of tilemaps, marking 
; touched tiles and redrawing
;
.module cpct_tilemaps

;;
;; Compilation control directives
;;
;; If true, interrupts can be disabled to get function parameters faster
.equ let_disable_interrupts_for_function_parameters, 0  

;;
;; Global values referring to the intergal tilemaps managing variables
;;
.globl _cpct_ptileset
.globl _cpct_pgetTileFunc
.globl _cpct_pgetTileIndexFunc
