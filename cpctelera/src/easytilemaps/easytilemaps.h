//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------

//
// Title: EasyTilemaps
//

#ifndef CPCT_EASYTILEMAPS_H
#define CPCT_EASYTILEMAPS_H

#include <types.h>

// EasyTilemaps managing functions
extern void cpct_etm_drawTilemap2x4_f(u8 map_width, u8 map_height, void* pvideomem, const void* ptilemap) __z88dk_callee;
extern void cpct_etm_drawTileBox2x4  (u8 x, u8 y, u8 w, u8 h, u8 map_width, void* pvideomem, const void* ptilemap) __z88dk_callee;
extern void cpct_etm_drawTileRow2x4  (u8 numtiles, void* video_memory, const void* ptilemap) __z88dk_callee;
extern void cpct_etm_setTileset2x4   (const void* ptileset) __z88dk_fastcall;

//
// Macro: cpct_etm_drawTilemap2x4
//
//    This macro uses <cpct_etm_drawTileBox2x4> to draw a complete tilemap. 
//
// C Definition:
//    #define <cpct_etm_drawTilemap2x4> (*W*, *H*, *SCR*, *TM*)
//
// Parameters (1 byte):
//  (1B) W   - Width  of the tilemap in tiles
//  (1B) H   - Height of the tilemap in tiles
//  (2B) SCR - Pointer to the screen or backbuffer location where tilemap will be drawn
//  (2B) TM  - Pointer to the tilemap 
//
// Details:
//    This macro draws a complete tilemap *TM* of size *W*, *H*, at the location *SCR* in 
// video memory or a backbuffer. It's main purpose is to simplify this operation, while 
// saving some space, as no other function is required for drawing the full tilemap.
//
//    Drawing a full tilemap could also be done much faster with <cpct_etm_drawTilemap2x4_f>,
// but that will include the code for that function in the final binary. If speed is not a
// great concern when drawing the full tilemap, using this macro is prefered to save some
// space.
//
#define cpct_etm_drawTilemap2x4(W, H, V, TM)    cpct_etm_drawTileBox2x4(0, 0, (W), (H), (W), (V), (TM))



#endif