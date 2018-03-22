//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include <map/tiles.h>
#include <map/court.h>

#define  VIEW_X_TILE    1
#define  VIEW_Y_TILE    2
#define  OFFSET         ((0x50 * VIEW_Y_TILE) + 4*VIEW_X_TILE)
#define  TILEMAP_VMEM   CPCT_VMEM_START + OFFSET

void initialize() {
   cpct_disableFirmware();
   cpct_setVideoMode(0);
   cpct_setPalette(g_palette, 16);
   cpct_setBorder(HW_BLACK);
}

void main(void) {
   initialize();

   cpct_etm_setDrawTilemap4x8_ag(g_courtTilemap_W, g_courtTilemap_H, g_courtTilemap_W, g_tiles_00);
   cpct_etm_drawTilemap4x8_ag(TILEMAP_VMEM, g_courtTilemap);

   while (1);
}
