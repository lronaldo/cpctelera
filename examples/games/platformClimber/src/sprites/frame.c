//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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
#include <types.h>
#include <cpctelera.h>
#include "frame.h"

////////////////////////////////////////////////////////////////////////////////////////////////
// Draw a frame around play area, using frame tiles.
//    pscr: Pointer to the upper-left corner of the screen / buffer where frame is to be drawn
//       x: X coordinate where the frame starts
//
void drawFrame(u8* pscr, u8 x) {
   u8 *pvmem;     // Pointer to video memory (used for drawing tiles)
   u8 i;          // Counter for loops

   //------ Draw Up-Left and Up-Right Corners
   pvmem = cpct_getScreenPtr(pscr, x, 0);
   cpct_drawTileAligned4x8(G_frameUpLeftCorner,  pvmem);       // Up-Left
   cpct_drawTileAligned4x8(G_frameUpRightCorner, pvmem + 54);  // Up-Right
   
   //------ Draw left and right borders
   for(i=8; i < 192; i += 8) {
      pvmem = cpct_getScreenPtr(pscr, x, i);
      cpct_drawTileAligned4x8(G_frameLeft,  pvmem);      // Left border
      cpct_drawTileAligned4x8(G_frameRight, pvmem + 54); // Right border
   }

   //------ Draw upper border
   pvmem = cpct_getScreenPtr(pscr, x, 0);
   for(i=4; i < 28; i += 4) {
      pvmem += 4;
      cpct_drawTileAligned4x8(G_frameUp, pvmem);       // Left part 
      cpct_drawTileAligned4x8(G_frameUp, pvmem + 26);  // Right part
   }
   cpct_drawSprite(G_frameUpCenter, pvmem + 2, 6, 8);  // Central tile
   
   //------ Draw down border
   pvmem = cpct_getScreenPtr(pscr, x, 192);
   for(i=4; i < 28; i += 4) {
      pvmem += 4;
      cpct_drawTileAligned4x8(G_frameDown, pvmem);       // Left part
      cpct_drawTileAligned4x8(G_frameDown, pvmem + 26);  // Right part
   }
   cpct_drawSprite(G_frameDownCenter, pvmem + 2, 6, 8);  // Central tile

   //------ Draw Down-Left and Down-Right Corners
   pvmem = cpct_getScreenPtr(pscr, x, 192);
   cpct_drawTileAligned4x8(G_frameDownLeftCorner,  pvmem);        // Down-Left
   cpct_drawTileAligned4x8(G_frameDownRightCorner, pvmem + 54);   // Down-Right
}
