//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "constants.h"
#include "map.h"
#include "cursor.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// PRIVATE GLOBAL VARIABLES 
///   Variables that are global for this module only
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Cursor Sprite definition (MASKED)
//    (Defines a red square with mask to be transparent)
const u8 cursor_sprite[8] = { 
   0x00, 0xFF, 
   0x66, 0x99, 
   0x66, 0x99, 
   0x00, 0xFF
};

// Cursor location
//    Tile coordinates of the cursor relative to the map
u8 cursor_x, cursor_y;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// FUNCTION DEFINITIONS
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// setLocation
//    Sets the cursor coordinates to the ones given (x,y)
//
void cursor_setLocation(u8 x, u8 y) {
   cursor_x = x; 
   cursor_y = y;
}

/////////////////////////////////////////////////////////////////
// draw
//    Draws the cursor at its current coordinates. It first 
// redraws the tile under the cursor to be sure that it is updated
// and will be seen through the transparent part of the cursor.
//
void cursor_draw() {
   u8 *pmem;

   // Draw the tile that lies at the current cursor location
   map_drawTile(cursor_x, cursor_y);

   // Calculate the memory location where the cursor is and draw
   // there the cursor sprite using transparent-mask drawing
   pmem = cpct_getScreenPtr(map_getBaseMem(), cursor_x, cursor_y*TILE_HEIGHT);
   cpct_drawSpriteMasked(cursor_sprite, pmem, TILE_WIDTH, TILE_HEIGHT);
}

/////////////////////////////////////////////////////////////////
// move
//    Remove cursor from its current location, move it and
// redraw it again in its new location.
//
void cursor_move(TMoveDir dir) {
   // Remove cursor from its current visual location by 
   // redrawing the tile that lies under it
   map_drawTile(cursor_x, cursor_y);
   
   // Update cursor location depending on the requested movement
   switch(dir) {
      case DIR_LEFT:  cursor_x--; break;
      case DIR_RIGHT: cursor_x++; break;
      case DIR_UP:    cursor_y--; break;
      case DIR_DOWN:  cursor_y++; break;
   }

   // Redraw the cursor at its new location
   cursor_draw();
}

/////////////////////////////////////////////////////////////////
// getX, getY
//    Returns X or Y coordinate of the cursor location
//
u8 cursor_getX() { return cursor_x; }
u8 cursor_getY() { return cursor_y; }