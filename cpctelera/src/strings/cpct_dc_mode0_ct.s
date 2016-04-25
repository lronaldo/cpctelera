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
.module cpct_strings

;;
;; Array: dc_mode0_ct 
;;
;;    Mode 0 Color conversion table (PEN to Screen pixel format)
;;
;;    This table converts PEN values (palette indexes from 0 to 15) into screen pixel format values in mode 0. 
;; In mode 0, each byte has 2 pixels (P0, P1). This table converts to Pixel 1 (P1) format. Getting values for
;; pixel 0 format only requires shifting the bits 1 to the left.
;;
dc_mode0_ct:: .db 0x00, 0x40, 0x04, 0x44, 0x10, 0x50, 0x14, 0x54, 0x01, 0x41, 0x05, 0x45, 0x11, 0x51, 0x15, 0x55

