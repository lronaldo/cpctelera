;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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

;;
;; Array: dc_mode1_ct 
;;
;;    Mode 1 Color conversion table (PEN to Screen pixel format)
;;
;;    This simple table converts PEN values (palette indexes from 0 to 3) into screen pixel format values in mode 1. 
;; In mode 1, each byte has 4 pixels (P0, P1, P2, P3). This table converts the PEN value to a byte that has the 4
;; pixels in the given PEN value colour. 
;;
dc_mode1_ct:: .db 0x00	;; PEN 0: All pixels are coloured 00
		  .db 0xF0	;; PEN 1: All pixels are coloured 01 (right nibble, 4 zeros for the 4 pixels, left nibble 4 ones)
		  .db 0x0F  ;; PEN 2: All 4-pixels are coloured 10 (similar to previous element, but inverted)
		  .db 0xFF	;; PEN 3: All 4-pixels are coloured 11, so all bits are 1.

