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
;; Array: cpct_pen2twoPixelM0_table 
;;
;;    Mode 0 PEN to 2-pixels-byte in screen mode format conversion table
;;
;;    This table associates the value of a Mode 0 PEN ([0-15]) with a byte encoding
;; 2-mode-0-pixels all of the PEN colour associated. For instance, for PEN 1, 
;; it associates 0xC0, which means 2-mode-0-pixels of PEN 1.
;;
cpct_pen2twoPixelM0_table:: .db 0x00, 0xC0, 0x0C, 0xCC, 0x030, 0xF0, 0x3C, 0xFC, 0x03, 0xC3, 0x0F, 0xCF, 0x33, 0xF3, 0x3F, 0xFF
