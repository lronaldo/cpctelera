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
const u8 g_palette[13] = { 
   0x14,    //  0 - Black
   0x0C,    //  6 - Bright Red
   0x00,    // 13 - While (Grey)
   0x06,    // 10 - Cyan
   0x1E,    // 12 - Yellow
   0x1C,    //  3 - Red
   0x16,    //  9 - Green
   0x12,    // 18 - Bright Green
   0x0B,    // 26 - Bright White
   0x0E,    // 15 - Orange
   0x03,    // 25 - Pastel Yellow
   0x04,    //  1 - Blue
   0x1F,    // 14 - Pastel Blue
};

/////////////////////////////////////////////////////////////////////////////////////////
// Tilemap: 2D matrix of tile-indexes (1-byte each). Each tile-index refers to the 
//          tile that occupies that index location inside the tileset array.
//
const u8 g_tilemap[MAP_WIDTH * MAP_HEIGHT] = {
   #include "tilemap.csv"
};
