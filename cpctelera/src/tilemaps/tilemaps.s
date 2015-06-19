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


;; MODULE: TileMaps
;; This module helps in the managements of tilemaps, marking 
;; touched tiles and redrawing
;;
.module cpct_tilemaps

;;
;; Variables: Tilemap Managed Structures and Functions
;;
;; These variables hold pointes to internal structures and functions 
;; required by tilemap managers. Each pointer is a 16-bits value and all
;; of them are initialized to 0x0000 by default.
;;
;; cpct_ptileset          - Pointer to the array containting the present set of tiles in use
;; cpct_ptilemap          - Pointer to the tilemap (2D array with indexes of tiles)
;; cpct_pgetTileFunc      - Pointer to the function that gets concrete tiles from the tileset given their indexes
;; cpct_pgetTileIndexFunc - Pointer to the function that gets tile indexes from the present tilemap
;;
_cpct_ptileset::          .dw #0000
_cpct_ptilemap::          .dw #0000
_cpct_pgetTileFunc::      .dw #0000
_cpct_pgetTileIndexFunc:: .dw #0000
