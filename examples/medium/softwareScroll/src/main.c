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
#include <video.h>
#include <maps/tileset.h>
#include <maps/building.h>
#include <maps/frame_updown.h>
#include <maps/frame_leftright.h>

#define  VIEWPORT_W  16
#define  VIEWPORT_H  16
#define  VIEW_X (2*4)
#define  VIEW_Y (4*8)
#define  OFF_X  (u16)(cpctm_screenPtr(0, VIEW_X, VIEW_Y))

void drawBuidlingScrolled(u16 offset) {
   u8* vmem = video_getBackBuffer();
   cpct_etm_drawTilemap4x8_ag(vmem + OFF_X, g_building + offset);  
   video_switchBuffers();
}

void drawFrame() {
   u8* vmem_buffer = video_getBackBuffer();
   u8* vmem;

   // UP-DOWN Frames
   cpct_etm_setDrawTilemap4x8_ag(20, g_frame_ud_H, g_frame_ud_W, g_tileset_00);
   cpct_etm_drawTilemap4x8_ag(vmem_buffer, g_frame_ud);
   vmem = cpct_getScreenPtr(vmem_buffer,  0*4, 20*8);
   cpct_etm_drawTilemap4x8_ag(vmem, g_frame_ud + 1);
   // Left-Right Frames
   cpct_etm_setDrawTilemap4x8_ag( 2, g_frame_lr_H, g_frame_lr_W, g_tileset_00);
   vmem = cpct_getScreenPtr(vmem_buffer,  0*4,  4*8);
   cpct_etm_drawTilemap4x8_ag(vmem, g_frame_lr);   
   vmem = cpct_getScreenPtr(vmem_buffer, 18*4,  4*8);
   cpct_etm_drawTilemap4x8_ag(vmem, g_frame_lr + 2);
}

void initialize() {
   cpct_disableFirmware();
   cpct_setVideoMode(0);
   cpct_setPalette(g_palette, 16);
   cpct_setBorder(HW_BLUE);

   video_initBuffers();
   drawFrame();
   video_switchBuffers();
   drawFrame();

   cpct_etm_setDrawTilemap4x8_ag(16, 16, g_building_W, g_tileset_00);
}

void game() {
   u16 offset=0;  // Offset in tiles of the start of the view window in the g_building tilemap
   u8  x=0, y=0;  // (x, y) coordinates of the start of the view window in the gbuilding tilemap

   // Loop forever
   while (1) {
      // Draw the tilemap scrolled to the current movement offset
      drawBuidlingScrolled(offset);

      // Check user input and update offset accordingly
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_O) && x)                             { --x; --offset; }
      if (cpct_isKeyPressed(Key_P) && x < g_building_W - VIEWPORT_W) { ++x; ++offset; }
      if (cpct_isKeyPressed(Key_Q) && y)                             { --y; offset -= g_building_W; }
      if (cpct_isKeyPressed(Key_A) && y < g_building_H - VIEWPORT_H) { ++y; offset += g_building_W; }
   }
}


void main(void) {
   cpct_setStackLocation((void*)0x8000);
   initialize();
   game();
}
