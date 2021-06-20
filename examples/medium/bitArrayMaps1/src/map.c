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

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// PRIVATE GLOBAL VARIABLES 
///   Variables that are global for this module only
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Pointer to the starting location of the map in video memory.
// This location is to be considered (0,0) coordinates in the map
//
u8 *map_base_location;

// MAPROW
//    Useful macro for defining the map. Each row of the map 
// consists in 80 bits, which is 10 bytes. So, we require 10 u8
// values to define 1 line. We could enter them directly in binary
// in the array like this: 0b01010101, 0b11001100,...,
// however, the "0b" prefixes would make it more difficulty to
// visually understand the map. This macro adds "0b" prefixes automatically
// to the 10 numbers that form 1 Row of the map. Therefore, we only
// have to type-in the 10 binary values, that will be closer and easier
// to understand.
//
#define MAPROW(C0,C1,C2,C3,C4,C5,C6,C7,C8,C9) 0b##C0,0b##C1,0b##C2,0b##C3,0b##C4,0b##C5,0b##C6,0b##C7,0b##C8,0b##C9

// MAP
//    Initial definition of the map as a CPCtelera bitarray.
// Each bit represents 1 tile that could only be 0(void) or 1(wall).
// The array has a total size of MAP_WIDTH * MAP_HEIGHT bits. We use
// the macro M (short alias for MAPROW, previously defined) to clarify 
// the contents of the map, that are defined in binary.
//
#define M MAPROW
const CPCT_1BITARRAY(map, MAP_WIDTH * MAP_HEIGHT) = {
   M(11111111,11111111,11111111,11111111,11111111,11111111,11111111,11111111,11111111,11111111),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000011,11110110,01011111,10111111,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,11000111,11000110,00110000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,11000111,11000110,00001100,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,11000110,01011111,10111111,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00001111,11011111,10000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000011,00001100,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000011,00000011,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00001111,11011111,10000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00110000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,01001000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,11111100,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,11001100,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00111000,00111110,11111101,11111000,00011001,10001100,01111110,00000000,00000011),
   M(11000000,01011000,00110100,00110000,01100000,00011111,10010010,01100110,00000000,00000011),
   M(11000000,00011000,00110010,00110000,01100000,00011001,10111111,01111110,00000000,00000011),
   M(11000000,01111110,00111100,11111100,01100000,00011001,10110011,01100000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000011),
   M(11111111,11111111,11111111,11111111,11111111,11111111,11111111,11111111,11111111,11111111)
};

// Remove macros that are no longer needed
#undef M
#undef MAPROW

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// FUNCTION DEFINITIONS
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Initialize map
//    Sets the base memory location for the map. This memory 
// location will be the place where tile (0,0) should be placed
//
void map_setBaseMem(u8* mem_location) {
   map_base_location = mem_location;
}

/////////////////////////////////////////////////////////////////
// getBaseMem
//    Returns the pointer to the base location of the map in 
// video memory. This location corresponds to tile (0,0).
//
u8* map_getBaseMem() { return map_base_location; }

/////////////////////////////////////////////////////////////////
// setTile
//    Sets a new value (0 or 1) for a concrete tile of the map.
// Coordinates are used to calculate the location of the tile
// in the map bitarray.
//
void map_setTile(u8 x, u8 y, u8 value) {
   // The map has MAP_HEIGHT rows, and each row has MAP_WIDTH
   // tiles. Therefore, location (x,y) is y times MAP_WIDTH
   // elements plus x elements.
   u16 tile_index = y * MAP_WIDTH + x;
   
   // Set the new value for (x,y) tile in the bitarray
   cpct_setBit(map, value, tile_index);
}

/////////////////////////////////////////////////////////////////
// getTile
//    Gets the value (0 or not-zero) of a concrete tile of the map.
// Coordinates refer to the concrete tile being addressed and are
// used to calculate the location of the tile in the map bitarray.
//
u8 map_getTile(u8 x, u8 y) {
   // The map has MAP_HEIGHT rows, and each row has MAP_WIDTH
   // tiles. Therefore, location (x,y) is y times MAP_WIDTH
   // elements plus x elements.
   u16 tile_index = y * MAP_WIDTH + x;
   
   // Get the value of the tile (x,y) from the bitarray and return it
   u8  tile_value = cpct_getBit(map, tile_index);
   return tile_value;
}


/////////////////////////////////////////////////////////////////
// drawTile
//    Draws only one of the tiles of the map, given its coordinates.
// It calculates the location in the screen of the given tile and
// gets its actual value from the map array. Then it draws the
// tile to the screen.
//
void map_drawTile(u8 x, u8 y) {
   // Calculate screen location where tile should be drawn
   u8 *pmem = cpct_getScreenPtr(map_base_location, x, y*TILE_HEIGHT);

   // Select appropriate colour pattern for the tile 
   // (YELLOW for walls, BLACK for background)
   u8 c_pattern = cpct_px2byteM1(C_BLACK, C_BLACK, C_BLACK, C_BLACK);
   if (map_getTile(x, y) != 0)
      c_pattern = cpct_px2byteM1(C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW);

   // Draw the tile as a solid box
   cpct_drawSolidBox(pmem, c_pattern, TILE_WIDTH, TILE_HEIGHT);   
}

/////////////////////////////////////////////////////////////////
// draw
//    Draws the map completely. It displays a message down the
// screen while drawing to inform the user.
//
void map_draw() {
   static u8* const string = "Drawing Map";
   u8 x, y;
   u8* pmem; 

   // Draw a message down the screen to inform the user
   // that the map is drawing
   pmem = cpct_getScreenPtr(CPCT_VMEM_START, 30, 160);
   cpct_setDrawCharM1(C_BLACK, C_RED);
   cpct_drawStringM1(string, pmem);

   // Draw the map tile by tile
   for(y=0; y < MAP_HEIGHT; y++) 
      for(x=0; x < MAP_WIDTH; x++) 
         map_drawTile(x, y);

   // Remove the message that was informing the user
   // (Drawing the string with background colour)
   cpct_setDrawCharM1(C_BLACK, C_BLACK);
   cpct_drawStringM1(string, pmem);
}

/////////////////////////////////////////////////////////////////
// changeTile
//    Interchanges the value of a tile (x,y). If the value of the
// tile is 0, it changes to 1. If the value is 1, it changes to 0.
//
void map_changeTile(u8 x, u8 y) {
   // Get present value of the tile (x, y)
   u8 tile = map_getTile(x, y);

   // If tile is        0, then tile = 1
   // If tile is not-zero, then tile = 0
   tile = (tile == 0);

   // Set the new value for the tile and redraw it
   map_setTile(x, y, tile);
   map_drawTile(x, y);
}

/////////////////////////////////////////////////////////////////
// clear
//    Fills in the map with 0 values, clearing it from walls. 
// After that, the map is redrawn to screen.
//
void map_clear() {
   u8 x, y;

   // Fill in the map with zeros (background)
   for(y=0; y < MAP_HEIGHT; y++) 
      for(x=0; x < MAP_WIDTH; x++) 
         map_setTile(x, y, 0);
   
   // Redraw the map
   map_draw();
}
