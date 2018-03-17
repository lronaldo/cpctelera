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

void drawTilemap(u8 x) {
   u8 y;
   u8* pvmem = buffer_ptr + 0xA0;
   u8* ptilem = g_tilemap + x;
   for(y=0; y < 10; y++) {
      cpct_etm_drawTileRow2x8_agf(20, ptilem, pvmem, g_tileset_0);
      pvmem += 0x50;
      ptilem += g_tilemap_W;
   }
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
   cpct_setBorder(HW_BLACK);
   buffer = 0x20;
   buffer_ptr = (u8*)(0x8000);
}

void main(void) {
   u8 x;
   cpct_setStackLocation((void*)0x8000);
   initialize();

   // Loop forever
   x = 0;
   while (1) {
      drawTilemap(x);
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_O) && x) --x;
      if (cpct_isKeyPressed(Key_P) && x < g_tilemap_W - 20) ++x;
   }
}
