//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C)      2015 Maximo / Cheesetea / ByteRealms (@rgallego87)
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
#include "spirit.h"

// Sprite size (in bytes)
#define SP_W   23
#define SP_H   54

// Screen size (in bytes)
#define SCR_W   80
#define SCR_H  200

// Floor location
#define FLOOR_Y      160
#define FLOOR_HEIGHT  10
#define FLOOR_COLOR  cpct_px2byteM0(1,2)

// Looking at right or left
#define LOOK_LEFT    0
#define LOOK_RIGHT   1

//
// MAIN: Using keyboard to move a sprite example
//
void initialize() {
   u8* pvideomem;               // Pointer to video memory

   // Disable firmware to prevent it from interfering with setPalette and setVideoMode
   cpct_disableFirmware();

   // Set up the hardware palette using hardware colour values
   cpct_setPalette(g_palette, 16);
   cpct_setBorder(HW_BLACK);
   
   // Set video mode 0 (160x200, 16 colours)
   cpct_setVideoMode(0);

   // Draw floor. As cpct_drawSolidBox cannot draw boxes wider than 63 bytes
   // and Screen width is 80 bytes, we draw 2 boxes of SCR_W/2 (40 bytes) each
   pvideomem = cpct_getScreenPtr(CPCT_VMEM_START,       0, FLOOR_Y);
   cpct_drawSolidBox(pvideomem, FLOOR_COLOR, SCR_W/2, FLOOR_HEIGHT);
   pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, SCR_W/2, FLOOR_Y);
   cpct_drawSolidBox(pvideomem, FLOOR_COLOR, SCR_W/2, FLOOR_HEIGHT);

   // Draw instructions
   pvideomem = cpct_getScreenPtr(CPCT_VMEM_START,  0, 20);
   cpct_drawStringM0("  Sprite Flip Demo  ", pvideomem, 2, 0);   
   pvideomem = cpct_getScreenPtr(CPCT_VMEM_START,  0, 34);
   cpct_drawStringM0("[Cursor]",   pvideomem, 4, 0);
   pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, 40, 34);
   cpct_drawStringM0("Left/Right", pvideomem, 3, 0);
}

//
// MAIN: Using keyboard to move a sprite example
//
void main(void) {
   u8  x=20;                    // Sprite horizontal coordinate
   u8  lookingAt = LOOK_RIGHT;  // Know where the sprite is looking at 
   u8* pvideomem;               // Pointer to video memory

   // Initialize the Amstrad CPC
   initialize();

   // 
   // Infinite moving loop
   //
   while(1) {
      // Scan Keyboard (fastest routine)
      // The Keyboard has to be scanned to obtain pressed / not pressed status of
      // every key before checking each individual key's status.
      cpct_scanKeyboard_f();

      // Check if user has pressed a Cursor Key and, if so, move the sprite if
      // it will still be inside screen boundaries
      if (cpct_isKeyPressed(Key_CursorRight) && x < (SCR_W - SP_W) ) {
         ++x;
         if (lookingAt == LOOK_LEFT) {
            lookingAt = LOOK_RIGHT;
            cpct_hflipSpriteM0(g_spirit, SP_W, SP_H);
         }
      } else if (cpct_isKeyPressed(Key_CursorLeft)  && x > 0) {
         --x; 
         if (lookingAt == LOOK_RIGHT) {
            lookingAt = LOOK_LEFT;
            cpct_hflipSpriteM0(g_spirit, SP_W, SP_H);
         }
      }
      
      // Get video memory byte for coordinates x, y of the sprite (in bytes)
      pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, x, FLOOR_Y - SP_H);

      // Draw the sprite in the video memory location got from coordinates x, y
      cpct_drawSprite(g_spirit, pvideomem, SP_W, SP_H);
   }
}
