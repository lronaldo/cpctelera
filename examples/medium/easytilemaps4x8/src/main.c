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
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include "tileset.h"
#include "tilemap.h"

u8* buffer_ptr;
u8  buffer;

#define  VIEW_X 20
#define  VIEW_Y 10
#define  OFF_X  (0xA0 + (20 - VIEW_X) * 2)

void drawTilemap(u16 offset) {
   cpct_etm_drawTileMap4x8_agf(buffer_ptr + OFF_X, g_tilemap + offset);
   cpct_waitVSYNC();
   cpct_setVideoMemoryPage(buffer);
   buffer     ^= 0x10;
   buffer_ptr = (u8*)((u16)(buffer_ptr) ^ 0x4000);
}

void initialize() {
   cpct_disableFirmware();
   cpct_memset((void*)0x8000, 0, 0x4000);
   cpct_setVideoMode(0);
   cpct_setPalette(g_palette, 16);
   cpct_setBorder(HW_WHITE);
   cpct_etm_setDrawTileMap4x8_agf(VIEW_X, VIEW_Y, g_tilemap_W, g_tileset_0);
   buffer = 0x20;
   buffer_ptr = (u8*)(0x8000);
}

void main(void) {
   u16 offset;
   u8  x, y;
   cpct_setStackLocation((void*)0x8000);
   initialize();

   // Loop forever
   offset = 0; x = y = 0;
   while (1) {
      drawTilemap(offset);
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_O) && x)                          { --x; --offset; }
      if (cpct_isKeyPressed(Key_P) && x < g_tilemap_W - VIEW_X)   { ++x; ++offset; }
      if (cpct_isKeyPressed(Key_Q) && y)                          { --y; offset -= g_tilemap_W; }
      if (cpct_isKeyPressed(Key_A) && y < g_tilemap_H - VIEW_Y)   { ++y; offset += g_tilemap_W; }
   }
}
