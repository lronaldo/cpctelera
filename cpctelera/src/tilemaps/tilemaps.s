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
;; cpct_tm_ptileset          - Pointer to the array containing the present set of tiles in use
;; cpct_tm_ptilemap          - Pointer to the tilemap (2D array with indexes of tiles)
;; cpct_tm_pgetTileFunc      - Pointer to the function that gets concrete tiles from the tileset given their indexes
;; cpct_tm_pgetTileIndexFunc - Pointer to the function that gets tile indexes from the present tilemap
;; cpct_tm_tilesize_x        - Horizontal size (in bytes) of a tile
;; cpct_tm_tilesize_y        - Vertical size (in bytes) of a tile. This size takes into account all the bytes that are between 
;;                             the start of a tile and the start of the next. For a standard screen, it should be 0x50*(pixels div 8) + 0x800*(pixels % 8)
;;
_cpct_tm_ptileset::          .dw #0000
_cpct_tm_ptilemap::          .dw #0000
_cpct_tm_pgetTileFunc::      .dw #0000
_cpct_tm_pgetTileIndexFunc:: .dw #0000
_cpct_tm_tilesize_x::        .db #00
_cpct_tm_tilesize_y::        .dw #0000
