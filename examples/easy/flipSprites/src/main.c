//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

///////////////////////////////////////////////////////////////////
// INCLUDE HEADERS
//
#include <cpctelera.h>  // Main cpctelera header
#include <sprite.h>     // Generated header in src/ folder after img/sprite.png automatic conversion

///////////////////////////////////////////////////////////////////
// DEFINE CONSTANTS
//
#define  LOOK_RIGHT    0   // Sprites look right by default, so no need to flip them (0, false) 
#define  LOOK_LEFT     1   // To make a sprite look left, it must be horizontally flipped (1, true)

///////////////////////////////////////////////////////////////////
// INITIALIZATION
//    Sets the initial configuration for the CPC
//
void initializeCPC() {
   cpct_disableFirmware();          // Disable firmware to prevent it from restoring mode and palette
   cpct_setVideoMode(0);            // Set video mode to 0 (160x200, 16 colours)
   cpct_setPalette(g_palette, 16);  // Set the palette using hardware values generated in sprite.h
   cpct_setBorder(HW_BLACK);        // Set border colour to Black (Same as background, or palette colour 0)
}

///////////////////////////////////////////////////////////////////
// DRAW A SPRITE
//    Draws a sprite at byte location (x, y) in the screen. The
// sprite will be drawn normal or horizontally flipped depending
// on the 'flipped' parameter (false: draw normal, true: draw flipped)
//
void drawSprite(u8 x, u8 y, u8 flipped) {
   // First calculate a pointer to (x, y) video memory byte location
   u8* pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
   
   // Draw the sprite at pvmem ((x,y) byte coordinates). Use HFlipM0 function
   // when to draw the sprite Horizontally flipped, and normal drawSprite to
   // draw the sprite normal. g_sprite, G_SPRITE_W and G_SPRITE_H are 
   // automatically generated in sprite.h, when converting img/sprite.png.
   // This automatic conversion is configured in cfg/image_conversion.mk
   //
   if (flipped) cpct_drawSpriteHFlipM0(g_sprite, pvmem, G_SPRITE_W, G_SPRITE_H);
   else         cpct_drawSprite       (g_sprite, pvmem, G_SPRITE_W, G_SPRITE_H);
}

///////////////////////////////////////////////////////////////////
// DRAW SCENE
//    Draws a simple scene with sprites drawn looking to the left
// and to the right.
//
void drawScene() {
   u8 y;    // Temporal variable for the y coordinate

   // Draw 5 sprite rows, with 4 sprites per row
   // Use constants LOOK_RIGHT (value 0, false) and LOOK_LEFT (value 1, true)
   // to draw each sprite flipped or normal. Each pair of sprites will be 
   // facing each other (one not flipped, next flipped)
   for(y=5; y < 200; y+=40) {
      drawSprite( 5, y, LOOK_RIGHT);   //  >>>>> (not flipped)
      drawSprite(25, y, LOOK_LEFT);    //  <<<<< (horizontally flipped)
      drawSprite(45, y, LOOK_RIGHT);   //  >>>>> (not flipped)
      drawSprite(65, y, LOOK_LEFT);    //  <<<<< (horizontally flipped)
   }
}

///////////////////////////////////////////////////////////////////
// MAIN PROGRAM
//
void main(void) {
   initializeCPC();  // First initialize the CPC
   drawScene();      // Then draw a simple scene 
   while (1);        // And wait forever
}
