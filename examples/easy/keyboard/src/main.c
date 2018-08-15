//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C)      2015 Maximo / Cheesetea / ByteRealms (@rgallego87)
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
#include "ctlogo.h"

// Sprite size (in bytes)
#define SP_W   12
#define SP_H   62

// Screen size (in bytes)
#define SCR_W   80
#define SCR_H  200

//
// MAIN: Using keyboard to move a sprite example
//
void main(void) {
   u8  x=10, y=10;   // Sprite coordinates
   u8* pvideomem;    // Pointer to video memory

   //
   // Set up the screen
   //
   // Disable firmware to prevent it from interfering with setPalette and setVideoMode
   cpct_disableFirmware();

   // Set the colour palette
   cpct_setPalette(G_palette, 4); // Set up the hardware palette using hardware colours
   
   // Set video mode 1 (320x200, 4 colours)
   cpct_setVideoMode(1);

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
      if      (cpct_isKeyPressed(Key_CursorRight) && x < (SCR_W - SP_W) ) ++x; 
      else if (cpct_isKeyPressed(Key_CursorLeft)  && x > 0              ) --x; 
      if      (cpct_isKeyPressed(Key_CursorUp)    && y > 0              ) --y;
      else if (cpct_isKeyPressed(Key_CursorDown)  && y < (SCR_H - SP_H) ) ++y;
      
      // Get video memory byte for coordinates x, y of the sprite (in bytes)
      pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);

      // Draw the sprite in the video memory location got from coordinates x, y
      cpct_drawSprite(G_ctlogo, pvideomem, SP_W, SP_H);
   }
}
