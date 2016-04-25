;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;;    This table converts PEN values (palette indexes from 0 to 4) into screen pixel format values in mode 1. 
;; In mode 1, each byte has 4 pixels (P0, P1, P2, P3). This table converts to Pixel 0 (P0) format. Getting values for
;; other pixels require shifting this byte to the right 1 to 3 times (depending on which pixel is required).
;;
dc_mode1_ct:: .db 0x00, 0x08, 0x80, 0x88

