//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2018 Miguel Sánchez aka PixelArtM (@PixelArtM)
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
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CPCTELERA EXAMPLE: DRAW A COURT
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// This example draws a court that is defined as a map of tiles (a tilemap).
// The map was created with Tiled (www.mapeditor.org): files img/court.tmx, img/court.tsx.
// The tiles are defined in img/tiles.png and can be edited with any graphics editor.
//
// The court is a tilemap composed of 8x8-pixels tiles (4x8 bytes in mode 0). 
// Tiles form a tileset (an array of tiles defined consecutively in memory), 
// at 4x8-bytes per tile. Tiles are generated as a list of arrays, one per tile,
// named g_tiles_XX (refer to src/maps/tiles.c to see them). As all of them are 
// defined consecutive in the same file, they will be also consecutive in memory, 
// as if they were a unique array. Therefore, g_tiles_00 is the address where the 
// first tile starts, and also were the tileset starts. Tiles generate from the 
// file img/tiles.png and produce the file src/maps/tiles.c. (This is controlled
// in the file cfg/image_conversion.mk)
//
// The tilemap maps which tile will be drawn at each location on the screen.
// For that, the tilemap is a 2D array of indexes (1-byte each). Each index refers
// to the tile that has to be drawn. This array is generated at src/map/court.c from
// the original img/court.tmx. This generation is controlled in the file 
// cfg/tilemap_conversion.mk.
//
// Court and Tiles are contributed by @PixelArtM (https://twitter.com/PixelArtM)
// and distributed under Creative Commons CC-BY-SA-3.0 License
//------------------------------------------------------------------------------

// Required Include files
#include <cpctelera.h>  // CPCtelera function declarations
#include <map/tiles.h>  // Tile declarations      (file generated after processing img/tiles.png)
#include <map/court.h>  // Court map declarations (file generated after processing img/court.tmx)

// Values that define the location where the Court will be drawn. The court is formed by 
// 18x22 8x8-tiles (144x176 pixels). As the Screen has 160x200 pixels in mode 0 (20x25 8x8-tiles), 
// to draw the court approximately centered, its upper-left tile must be at tile (1,2) in the 
// screen (1 tile to the right, 2 to the downside). Each tile has a size of 4x8-bytes in mode 0 
// (that is, 8x8 pixels). With that, we can calculate the screen location, using the macro 
// cpctm_screenPtr (As all values are constants, the macro won't generate any calculation code. 
// Calculations will be performed on compilation and a memory address will be the result).
//
// DON'T FORGET that selected location MUST BE a character pixel line 0 (0xC000 to 0xC7FF in
// standard video modes)
//
// The size of a tile is 4x8-bytes
#define  TILESIZE_X     4
#define  TILESIZE_Y     8

// Upper-left tile will be at (1,2) coordinate in tiles. Convert to byte coordinates.
#define  VIEWPORT_X     (1*TILESIZE_X)
#define  VIEWPORT_Y     (2*TILESIZE_Y)

// Convert byte coordinates to video memory address (all values constant, use macro)
#define  TILEMAP_VMEM   cpctm_screenPtr(CPCT_VMEM_START, VIEWPORT_X, VIEWPORT_Y)

////////////////////////////////////////////////////////////////////////////////////
// INITIALIZE THE CPC
//
void initialize() {
   // Firmware must be disabled to be able to change video mode,
   // setting palette colours and border
   cpct_disableFirmware();          // Disable firmware 
   cpct_setVideoMode(0);            // Set Video Mode 0 (160x200, 16 colours)
   cpct_setPalette(g_palette, 16);  // Set palette colours (g_palette is generated in tiles.c)
   cpct_setBorder(HW_BLACK);        // Set border colour to black
}

////////////////////////////////////////////////////////////////////////////////////
// MAIN CODE FOR THIS EXAMPLE
//    Just initialize the CPC, draw the court map and wait forever
//
void main(void) {
   initialize();  // Initialize the CPC

   // DRAW THE COURT

   // 1) Use setDrawTilemap4x8_ag to configure drawTilemap4x8_ag internal values.
   // First 2 values are the size of the window to be drawn in tiles (all the court map, 
   // so g_courtMap_W x g_courtMap_H or 18x22). Next value is the width of the complete 
   // tilemap in tiles (g_courtMap_W or 18). Last value is the address where the tileset
   // definition starts (g_tiles_00 is the address where the definition of the first
   // tile starts, and next tiles are defined consecutively in memory)
   cpct_etm_setDrawTilemap4x8_ag(g_courtMap_W, g_courtMap_H, g_courtMap_W, g_tiles_00);

   // 2) After configuring values for drawTilemap4x8_ag function, we only need to call it
   // each time we wanted to draw the tilemap. It only needs to now 2 things: location
   // in video memory where to draw the tilemap (we calculated it before: TILEMAP_VMEM),
   // and a pointer to the first tile in the tilemap to be drawn (As we want to draw the
   // whole tilemap, the first tile to be drawn is the first of the complete tilemap, and
   // its address is where the tilemap starts, so g_courtMap).
   cpct_etm_drawTilemap4x8_ag(TILEMAP_VMEM, g_courtMap);

   // Loop forever
   while (1);
}
