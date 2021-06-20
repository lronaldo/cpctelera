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
#include "sprites/sprites.h"

// Background information
// Starting screen coordinates of the top-left pixel (in bytes)
// Width and Height of the background (in tiles)
#define BACK_X  12
#define BACK_Y  72
#define BACK_W  14
#define BACK_H   6

// Sprite size (in bytes)
#define SPR_W    4
#define SPR_H   16

// Cycles to wait between sprite movements
#define WAITLOOPS 4000

/////////////////////////////////////////////////////////////////////////
// Draws a frame box around the "play zone"
//
void drawFrame() {
  u8* pvmem;    // Pointer to video memory for drawing boxes
  u8  pattern;  // Colour pattern to be drawn

  // Colour pattern for frame boxes (2 pixels of PEN colour 15)
  pattern = cpct_px2byteM0 (15, 15);
  
  // Draw top box
  pvmem = cpct_getScreenPtr(CPCT_VMEM_START, (BACK_X),  (BACK_Y - 8) );
  cpct_drawSolidBox(pvmem, pattern, 4*BACK_W,  8);

  // Draw bottom box
  pvmem = cpct_getScreenPtr(CPCT_VMEM_START, (BACK_X),  (BACK_Y + 8*BACK_H) );
  cpct_drawSolidBox(pvmem, pattern, 4*BACK_W,  8);

  // Draw left box
  pvmem = cpct_getScreenPtr(CPCT_VMEM_START, (BACK_X - 4), (BACK_Y - 8) );
  cpct_drawSolidBox(pvmem, pattern,  4, 8*(BACK_H + 2) );

  // Draw right box
  pvmem = cpct_getScreenPtr(CPCT_VMEM_START, (BACK_X + 4*BACK_W),  (BACK_Y - 8));
  cpct_drawSolidBox(pvmem, pattern,  4, 8*(BACK_H + 2) );
}

/////////////////////////////////////////////////////////////////////////
// Draws the complete tiled background
//    It paints all the tiles in the tile_array that defines the 
// background. That is the same as drawing the complete background.
//
void drawBackground() {
   u8   x, y;       // Column and Row in the tile array of the next tile to draw
   u8 *pvideomem;   // Pointer to video memory, where the tile will be drawn

   // Draw the complete background (BACK_W * BACK_H tiles)
   
   // Traverse rows of the tile array
   for (y=0; y < BACK_H; ++y) {     

      // Point to the video memory byte where tiles of this row should start.
      // X coordinate is always the same (first X coordinate, or BACK_X) and
      // y coordinate increments by 8 with each row.
      pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, BACK_X, BACK_Y + 8*y);

      // Traverse columns of the tile array
      for(x=0; x < BACK_W; ++x) {
         cpct_drawTileAligned4x8_f(G_background[x][y], pvideomem);  // Draw next tile
         
         // Point 4 bytes to the right, to the next place for tile 
         // (as tiles are 4 bytes wide)
         pvideomem += 4;  
      }
   }
}

/////////////////////////////////////////////////////////////////////////
// Repaints the background over an sprite to "erase" it
//    It uses (x, y) screen byte coordinates of the sprite to know the
// 4 tiles it will be "touching" and then repaints them 4. This is only
// valid for 8x16 pixels sprites (4x16 bytes)
//
void repaintBackgroundOverSprite(u8 x, u8 y) {
   u8* pvmem;                     // Pointer to video memory for redrawing tiles
   
   // Calculate the tile in which is place the top-left corner of the sprite
   // (x,y) coordinate refer to that point in bytes
   u8 tilex = (x - BACK_X) / 4;   // Calculate tile column into the tile array (integer division, 4 bytes per tile)
   u8 tiley = (y - BACK_Y) / 8;   // Calculate tile row into the tile array (integer division, 8 bytes per tile)
   u8 scrx = BACK_X + 4*tilex;    // Calculate x screen byte coordinate of the tile
   u8 scry = BACK_Y + 8*tiley;    // Calculate y screen byte coordinate of the tile
   
   // Now we have the tile in which top-left corner of our sprite is placed.
   // Our sprite is 4x16 bytes wide, so it can extend up to 1 tile to the right
   // and up to 2 tiles downside. However, as our sprite always moves in X, it will
   // always be aligned and will only extend 1 tile downside. So, drawing 4 tiles
   // is enough to be sure that we are erasing the sprite and restoring the background.
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, scrx, scry);
   cpct_drawTileAligned4x8_f(G_background[tilex  ][tiley], pvmem    );
   cpct_drawTileAligned4x8_f(G_background[tilex+1][tiley], pvmem + 4);
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, scrx, scry + 8);
   cpct_drawTileAligned4x8_f(G_background[tilex  ][tiley+1], pvmem    );
   cpct_drawTileAligned4x8_f(G_background[tilex+1][tiley+1], pvmem + 4);
}

/////////////////////////////////////////////////////////////////////////
// Initialization routine
//    Disables firmware, initializes palette and video mode and
// draws the background
//
void initialization (){ 
   cpct_disableFirmware();          // Disable firmware to prevent it from interfering
   cpct_fw2hw     (G_palette, 16);  // Convert firmware colours to hardware colours 
   cpct_setPalette(G_palette, 16);  // Set palette using hardware colour values
   cpct_setBorder (G_palette[0]);   // Set border colour same as background (0)
   cpct_setVideoMode(0);            // Change to Mode 0 (160x200, 16 colours)

   drawFrame();       // Draw a Frame around the "play zone"
   drawBackground();  // Draw the tiled background
}

void main(void) {
   u8 x=BACK_X+1;   // x byte screen coord. of the sprite (1 byte to the right of the start of the "play zone")
   u8 y=BACK_Y+4*8; // y byte screen coord. of the sprite (4 tiles down the start of the "play zone")
   i8 vx=1;         // Horizontal movement velocity in bytes (1 byte to the right)
   u8* pvmem;       // Pointer to video memory for drawing the sprite
   u16 i;           // Counter used in the wait loop (16 bits, up to 65535)

   // Initialize screen, palette and background
   initialization();

   //
   // Main loop: Moves the sprite left-to-right and vice-versa
   //
   while(1) {
      // Draw the sprite with Mask. Calculate screen byte where to byte it
      // and call drawSpriteMasked to ensure that the sprite is drawn without
      // erasing the background. This is only valid for sprite defined with mask.
      pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);      
      cpct_drawSpriteMasked(G_sprite_EMR, pvmem, SPR_W, SPR_H);

      for(i=0; i < WAITLOOPS; i++); // Wait for a little while
      cpct_waitVSYNC();             // Synchronize with VSYNC to prevent flickering
      
      repaintBackgroundOverSprite(x, y); // Repaint the background only where the sprite is located
      
      // Move the sprite using its velocity (Just add vx to current x position)
      x += vx;

      // Check if we have crossed boundaries and, if it is the case,
      // position sprite again where it was previously and change velocity sense.
      if (x < BACK_X || x > (BACK_X + 4*BACK_W-5) ) {
        x -= vx;    // Undo latest movement subtracting vx from current x position
        vx = -vx;   // Change the sense of velocity to start moving opposite

        // Optionally, Sprite may be flipped to look backwards
        //cpct_hflipSpriteMaskedM0(SPR_W, SPR_H, G_sprite_EMR);
      }
   }
}
