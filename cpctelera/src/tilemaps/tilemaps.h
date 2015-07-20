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
//-------------------------------------------------------------------------------

//
// Title: Tilemaps
//


#ifndef CPCT_TILEMAPS_H
#define CPCT_TILEMAPS_H

#include <types.h>

// EASY TILEMAPS
//
typedef struct {
   void *ptilemap;
   void *ptileset;
   void *pscreen;
   u8   map_height;
   u8   map_width;
   u8   tile_size;
} cpct_TEasyTilemap;

extern void cpct_etm_drawFullTilemap(cpct_TEasyTilemap* tilemap) __z88dk_fastcall;
extern void cpct_etm_redrawTilemap(cpct_TEasyTilemap* tilemap) __z88dk_fastcall;

// Setting global configuration for tilemanagement
extern void cpct_tm_setTileset              (void* ptileset);
extern void cpct_tm_setTilemap              (void* ptilemap);
extern void cpct_tm_setFunctionGetTileIndex (void* pgetTileIndexFunc);
extern void cpct_tm_setFunctionGetTile      (void* pgetTileFunc);

#endif