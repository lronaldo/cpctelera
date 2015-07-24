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

//
// Type: cpct_TEasyTilemap
//
//    Structure that contains all the information about an easy Tilemap.
//
// C Definition:
//    typedef struct {
//       void *ptilemap;   // Pointer to the 2D tile-index matrix (the tilemap)
//       void *ptileset;   // Pointer to the array of pointers to tile definitions (2x4-sized sprites)
//       void *pscreen;    // Pointer to the location where the tilemap is to be drawn
//       u8   map_height;  // Height of the tilemap in tiles
//       u8   map_width;   // Width of the tilemap in tiles
//    } cpct_TEasyTilemap;
//
// Details:
//   This structure holds all the required information about an EasyTilemap. The user must
// create and populate this structure and then it can be passed to tilemap managing 
// functions for drawing the tilemap.
//
typedef struct {
   void *ptilemap;   // Pointer to the 2D tile-index matrix (the tilemap)
   void *ptileset;   // Pointer to the array of pointers to tile definitions (2x4-sized sprites)
   void *pscreen;    // Pointer to the location where the tilemap is to be drawn
   u8   map_height;  // Height of the tilemap in tiles
   u8   map_width;   // Width of the tilemap in tiles
} cpct_TEasyTilemap;

// Easytilemap managing functions
extern void cpct_etm_drawFullTilemap(const cpct_TEasyTilemap* tilemap) __z88dk_fastcall;
extern void cpct_etm_redrawTileBox  (const void* ptilemap, u8 x, u8 y, u8 w, u8 h) __z88dk_callee;
extern void cpct_etm_drawTileRow    (u8 numtiles, const void* video_memory, const void* ptilemap) __z88dk_callee;
extern void cpct_etm_setTileset     (const void* ptileset) __z88dk_fastcall;

#endif