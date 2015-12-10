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

#include <types.h>
#include "tiles.h"  // Generated file including tiles and tileset

// Define constant sizes
#define TILESIZE      2*4
#define NUMTILES        4
#define SCR_TILE_WIDTH 40
#define MAP_WIDTH     120
#define MAP_HEIGHT     48
#define SCR_HEIGHT    200
#define SCR_WIDTH      80

// Define Palette
extern const u8 g_palette[4];

// Declaration of the tilemap
extern const u8 g_tilemap[MAP_WIDTH * MAP_HEIGHT];