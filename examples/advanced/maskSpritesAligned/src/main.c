//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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
#include "sprites/tiles.h"
#include "sprites/map.h"
#include "sprites/alien.h"

// Sets the transparent mask table for color 0, mode 0
cpctm_createTransparentMaskTable(g_masktable, 0x0100, M0, 0);

// Some useful constants
#define MAP_WIDTH_TILES          40
#define MAP_HEIGHT_TILES         50
#define ALIEN_WIDTH_BYTES         6
#define ALIEN_HEIGHT_BYTES       24
#define ALIEN_WIDTH_TILES         3
#define ALIEN_HEIGHT_TILES        6
#define TILEWIDTH_BYTES           2
#define TILEHEIGHT_BYTES          4

//
// Struct for defining the location of an alien 
//
typedef struct {
   u8 tx, ty;     // Location on the screen (in tiles)
   i8 vx, vy;     // Movement velocity      (in tiles)
} TAlien;

/////////////////////////////////////////////////////////////////////////
// Initialization routine
//    Disables firmware, initializes palette and video mode and
// draws the background
//
void initialization (){ 
   cpct_disableFirmware();          // Disable firmware to prevent it from interfering
   cpct_setPalette(g_palette, 7);   // Set palette using hardware colour values
   cpct_setBorder (HW_BLACK);       // Set border colour same as background (Black)
   cpct_setVideoMode(0);            // Change to Mode 0 (160x200, 16 colours)

   // Set the internal tileset for drawing Tilemaps
   cpct_etm_setTileset2x4(g_tileset);

   // Draw the background tilemap
   cpct_etm_drawTilemap2x4_f(MAP_WIDTH_TILES, MAP_HEIGHT_TILES, 
                             CPCT_VMEM_START, g_background);  
}

////////////////////////////////////////////////////////////////////////////////////////
// Wait for n VSYNCs
//
void waitNVSYNCs(u8 n) {
   do {
      cpct_waitVSYNC();
      if (--n) {
         __asm__ ("halt");
         __asm__ ("halt");
      }
   } while (n);
}

////////////////////////////////////////////////////////////////////////////////////////
// MAIN PROGRAM: move a sprite over a background
//
void main(void) {
   // One alien that will move bouncing through the screen
   static const TAlien sa = {0, 0, 1, 1};
   TAlien* a = (void*)&sa;

   // Initialize screen, palette and background
   initialization();

   //
   // Main loop: Moves the sprite left-to-right and vice-versa
   //
   while(1) {
      u8* pscra;  // Pointer to the screen location to draw the alien

      // Check if sprite is going to got out the screen and produce bouncing in its case
      if (a->vx < 0) {
         if (a->tx < -a->vx)
            a->vx = 1;
      } else if (a->tx + a->vx + ALIEN_WIDTH_TILES >= MAP_WIDTH_TILES)
         a->vx = -1;

      if (a->vy < 0) {
         if (a->ty < -a->vy)
            a->vy = 1;
      } else if (a->ty + a->vy + ALIEN_HEIGHT_TILES >= MAP_HEIGHT_TILES)
         a->vy = -1;

      // Wait for VSYNC before drawing to the screen to reduce flickering
      // We also wait for several VSYNC to make it move slow, or it will be too fast
      waitNVSYNCs(2);

      // Redraw a tilebox over the alien to erase it (redrawing background over it)
      cpct_etm_drawTileBox2x4(a->tx, a->ty, ALIEN_WIDTH_TILES, ALIEN_HEIGHT_TILES, 
                              MAP_WIDTH_TILES, CPCT_VMEM_START, g_background);
      // Move the alien and calculate it's new location on screen
      a->tx += a->vx; 
      a->ty += a->vy;
      pscra = cpct_getScreenPtr(CPCT_VMEM_START, TILEWIDTH_BYTES*a->tx, TILEHEIGHT_BYTES*a->ty);
      // Draw the alien in its new location
      cpct_drawSpriteMaskedAlignedTable(g_alien, pscra, ALIEN_WIDTH_BYTES, 
                                        ALIEN_HEIGHT_BYTES, g_masktable);
   }
}
