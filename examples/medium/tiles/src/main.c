//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "sprites.h"

// 4 names for our 4 types of drawSprite functions
typedef enum { _2x8, _4x8, _2x4Fast, _4x4Fast, _2x8Fast, _4x8Fast } TDrawFunc;

// Structure grouping all the information required about a tile for this example
//    ( pixel data, width in bytes and function to draw the tile)
typedef struct {
         u8* sprite;   // Pixel data defining the tile
         u8  width;    // Width in bytes of the tile
         u8  height;   // Height in bytes of the tile
   TDrawFunc function; // Function that should be used to draw this tile
} TTile;

// Constants used to control length of waiting time
const u16 WAITCLEARED = 20000; 
const u16 WAITPAINTED = 60000; 

// Table with all the tiles and the required information to draw them
const TTile tiles[6] = {
//    Sprite    Width Height   Function
//-----------------------------------------
   { waves_2x4,   2,    4,      _2x4Fast },  // Tile 0
   { waves_4x4,   4,    4,      _4x4Fast },  // Tile 0
   { waves_2x8,   2,    8,      _2x8     },  // Tile 1
   {     F_2x8,   2,    8,      _2x8Fast },  // Tile 2
   { waves_4x8,   4,    8,      _4x8     },  // Tile 3
   {    FF_4x8,   4,    8,      _4x8Fast }   // Tile 4
};

//
// Fills all the screen with sprites using drawTileAlignedXXX functions
//
void fillupScreen(TTile* tile) {
   u8 *pvideomem;                      // Pointer to the place where next tile will be drawn
   u8 x, y;                            // Loop counters for x and y screen tiles
   u8 tilesperline = 80/tile->width;   // Number of tiles per line = LINEWIDTH / TILEWIDTH

   // Cover all the screen (200 pixels) with tiles
   for (y=0; y < 200; y += tile->height) { 
      pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, 0, y); // Calculate byte there this pixel line starts

      // Draw all the tiles for this line
      for (x=0; x < tilesperline; x++) {       
         // Select the appropriate function to draw the tile, and draw it
         switch (tile->function) {
            case _2x4Fast: cpct_drawTileAligned2x4_f(tile->sprite, pvideomem); break;
            case _4x4Fast: cpct_drawTileAligned4x4_f(tile->sprite, pvideomem); break;
            case _2x8Fast: cpct_drawTileAligned2x8_f(tile->sprite, pvideomem); break;
            case _2x8:     cpct_drawTileAligned2x8  (tile->sprite, pvideomem); break;
            case _4x8Fast: cpct_drawTileAligned4x8_f(tile->sprite, pvideomem); break;
            case _4x8:     cpct_drawTileAligned4x8  (tile->sprite, pvideomem); break;
         }

         // Increase video memory pointer by tile->width bytes (point to next tile's place)
         pvideomem += tile->width;
      }
   }
}

//
// MAIN LOOP
//
void main(void) {
   // Initialization
   cpct_disableFirmware();
   cpct_setVideoMode(0);

   // Main loop: filling the screen using the 4 different basic 
   //            aligned functions in turns.
   while(1) {
      u8  i;   // Loop counters
      u16 w;

      // 4 iterations of filling up the screen out of tiles using
      // the 4 different tile-drawing functions
      for (i=0; i < 6; i++) {
         // First, clear the screen and wait for a while
         cpct_clearScreen(0);
         for (w=0; w < WAITCLEARED; w++);

         // Then, fill up the screen with the next tile-drawing function and wait another while
         fillupScreen(&(tiles[i]));
         for (w=0; w < WAITPAINTED; w++);
      }
   }
}
