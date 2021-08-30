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
;; Array: cpct_pen2fourPixelM1_table 
;;
;;    Mode 1 PEN to 4-pixels-byte in screen mode format conversion table
;;
;;    This table associates the value of a Mode 1 PEN ([0-3]) with a byte encoding
;; 4-mode-1-pixels all of the PEN colour associated. For instance, for PEN 1, 
;; it associates 0xF0, which means 4-mode-1-pixels of PEN 1.
;;
cpct_pen2fourPixelM1_table:: .db 0x00, 0xF0, 0x0F, 0xFF