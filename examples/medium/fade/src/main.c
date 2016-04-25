//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 SunWay / Fremos / Carlio 
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
#include "modules/sprites.h" // Pixel data definitions of the sprites and TSprite structure
#include "modules/palette.h" // Palette functions: fade-in, fade-out and RGB-HW colour conversions
#include "modules/utils.h"   // Function wait_frames 

//
// MAIN: Palette Effects Example
//
void main(void) {
  // Create a table with the 3 sprites that will be used
  const TSprite img[3] = { 
    { G_Goku,   30, 75, 20, 49 }, // Goku sprite centered   (size = 20x49 bytes = 40x49 pixels)
    { G_Vegeta, 22, 60, 36, 80 }, // Vegeta sprite centered (size = 36x80 bytes = 72x80 pixels)
    { G_No13,   22, 60, 36, 80 }  // No13 sprite centered   (size = 36x80 bytes = 72x80 pixels)
  };
  u8* pvmem;  // Pointer to video memory where sprites will be drawn
  u8  i;      // Index variable to iterate through the sprites

  // Initialize the CPC
  cpct_disableFirmware();   // Disable firmware to prevent it from interfering
  cpct_setVideoMode(0);     // Set video mode 0 (160x200, 16 colours)
  setBlackPalette(0, 16);   // Set all 17 colours (16 palette + border) to Black

  //
  // Infinite Loop
  //
  while(1) {

    // Iterate through the 3 sprites using fade in / fade out palette effect
    for(i=0; i < 3; ++i) {
      cpct_clearScreen(0x00);   // Clear the screen filling it up with 0's
      
      // Calculate video memory location for next sprite and draw it
      pvmem = cpct_getScreenPtr(CPCT_VMEM_START, img[i].x, img[i].y);
      cpct_drawSprite(img[i].sprite, pvmem, img[i].w, img[i].h);

      wait_frames(50);                    // Wait 1 second  ( 50 VSYNCs)
      fade_in (G_rgb_palette, 0, 16, 4);  // Do a Fade in effect to show the sprite
      
      wait_frames(100);                   // Wait 2 seconds (100 VSYNCs)
      fade_out(G_rgb_palette, 0, 16, 4);  // Do a Fade out effect to return to black
    }
  }
}
