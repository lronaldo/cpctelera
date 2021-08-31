;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_sprites

;;
;; ASM bindings for <cpct_pen2pixelPatternM1>
;;
;;    5 microSecs, 2 bytes
;;
cpct_pen2pixelPatternM1_asm::    ;; Assembly entry point
   ;; Include common code
   .include /cpct_pen2pixelPatternM1.asm/

   ld     a, (hl)    ;; [2] A = 4-pixels-mode-1-byte with all pixels in Pen colour
   ret               ;; [3] Return