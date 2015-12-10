//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include "tilemap.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Palette: Global colours used in this example
//
const u8 g_palette[4] = { 
   0x14,    // 0  - White (Grey)
   0x00,    // 13 - Black
   0x0A,    // 26 - Bright Yellow
   0x1C     // 3  - Red
};

/////////////////////////////////////////////////////////////////////////////////////////
// Tilemap: 2D matrix of tile-indexes (1-byte each). Each tile-index refers to the 
//          tile that occupies that index location inside the tileset array.
//
const u8 g_tilemap[MAP_WIDTH * MAP_HEIGHT] = {
   #include "tilemap.csv"
};
