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

#include <cpctelera.h>
#include "mazes/mazes.h"
#include "sprites/sprites.h"
#include "entities.h"

// palette = 
const u8 g_palette[9] = { 
// HW    //  FW (Colour)
//-----------------------
   0x14, //   0 (Black)
   0x15, //   2 (Bright Blue)
   0x0C, //   6 (Bright Red)
   0x0D, //   8 (Bright Magenta)
   0x1E, //  12 (Yellow)
   0x07, //  16 (Pink)
   0x13, //  20 (Bright Cyan)
   0x0A, //  24 (Bright Yellow)
   0x0B  //  26 (White)
};

// We always draw over the backbuffer
u8* const g_backBuffer = (u8*)0x8000;
TEntity* g_player;

//////////////////////////////////////////////////////////////
// Switch Front and Back Screen Buffers
//   Backbuffer is shown on screen,
//   Frontbuffer is removed from screen and treated as new backbuffer
//
void switchScreenBuffers() {
   u8** backbuf = (u8**)(&g_backBuffer);

   // Check which is present backbuffer and then switch
   if (g_backBuffer == (u8*)0x8000) {
      *backbuf = (u8*)0xC000;
      cpct_setVideoMemoryPage(cpct_page80);
   } else {
      *backbuf = (u8*)0x8000;
      cpct_setVideoMemoryPage(cpct_pageC0);
   }
}

u8 readUserInput() {
   cpct_scanKeyboard(); // Scan and update keyboard information to its present status

   // Only check infividual keys if at least one is pressed
   if (cpct_isAnyKeyPressed()) {
      if (cpct_isKeyPressed(Key_CursorLeft))
         ent_move(g_player, -1,  0);
      else if (cpct_isKeyPressed(Key_CursorRight))
         ent_move(g_player,  1,  0);
      else if (cpct_isKeyPressed(Key_CursorUp))
         ent_move(g_player,  0, -1);
      else if (cpct_isKeyPressed(Key_CursorDown))
         ent_move(g_player,  0,  1);

      return 1;
   }

   return 0;
}

void redrawScreen() {
   maze_draw(g_backBuffer);
   ent_drawAll(g_backBuffer);
   cpct_waitVSYNC();
   switchScreenBuffers();   
}

void game(){
   maze_initialize(0);
   ent_initialize();
   g_player = ent_getEntity(0);

   // Loop forever
   redrawScreen();
   while (1) {
      if (readUserInput()) {
         // Check limits
         if (!g_player->tx) {
            maze_moveTo(MM_LEFT);
            g_player->tx = 36;
         } else if (g_player->tx > 36) {
            maze_moveTo(MM_RIGHT);
            g_player->tx = 1;
         } else if (!g_player->ty) {
            maze_moveTo(MM_UP);
            g_player->ty = 46;
         } else if (g_player->ty > 46) {
            maze_moveTo(MM_DOWN);
            g_player->ty = 1;
         }

         redrawScreen();
      }
   }
}

void main(void) {
   cpct_setStackLocation((u8*)0x8000);
   cpct_disableFirmware();
   cpct_setVideoMode(0);
   cpct_setPalette(g_palette, 9);
   cpct_setBorder(g_palette[0]);

   game();
}
