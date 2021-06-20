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

#include <cpctelera.h>
#include "sprites.h"

// Death sprite Width and Height in _bytes_. In Mode 1, 1 Byte = 8 pixels,
// ...then 36 x 44 pixels = 9 x 44 bytes.
#define SPR_W  9
#define SPR_H  44

//
// EXAMPLE: Measuring free microseconds per frame after drawing a sprite
//
void main(void) {
   u8  i;                           // Loop index
   u8  x=0, y=0;                    // Sprite coordinates (in bytes)
   u8* pvideomem = CPCT_VMEM_START; // Sprite initial video memory byte location (where it will be drawn)
   u16 ms = 0;                      // Available microseconds until next VSYNC, after all main loop calculations

   // First, disable firmware to prevent it from intercepting our palette and video mode settings (and,
   // at the same time, winning some speed not having to process firmware code at every interrupt)
   cpct_disableFirmware();
   // Set palette and Screen Border (transform firmware to hardware colours and then set them)
   cpct_fw2hw     (G_palette, 4);
   cpct_setPalette(G_palette, 4);
   cpct_setBorder (G_palette[1]);
   cpct_setVideoMode(1);         // Ensure MODE 1 is set
   cpct_setDrawCharM1(3, 0);     // Always draw characters using same colours (3 (Yellow) / 0 (Grey))

   // Main Loop
   while(1) {
      // Scan Keyboard and change sprite location if cursor keys are pressed
      cpct_scanKeyboard_f();
      if      (cpct_isKeyPressed(Key_CursorRight) && x <  80 - SPR_W) { x++; pvideomem++; }
      else if (cpct_isKeyPressed(Key_CursorLeft)  && x >   0        ) { x--; pvideomem--; }
      if      (cpct_isKeyPressed(Key_CursorUp)    && y >   0        ) { pvideomem -= (y-- & 7) ? 0x0800 : 0xC850; }
      else if (cpct_isKeyPressed(Key_CursorDown)  && y < 200 - SPR_H) { pvideomem += (++y & 7) ? 0x0800 : 0xC850; }

      // Wait VSYNC monitor signal to synchronize the loop with it. We'll start drawing the sprite
      // calculations always at the same time (when VSYNC is first detected)
      cpct_waitVSYNC();

      // Draw the sprite at its new location on screen. 
      // Sprite automatically erases previous copy of itself on the screen because it moves 
      // 1 byte at a time and has a 0x00 border that overwrites previous colours on that place
      cpct_drawSprite(G_death, pvideomem, SPR_W, SPR_H);
      
      // Wait to next VSYNC signal, calculating the amount of free microseconds (time we wait for VSYNC)
      // As documented on <cpct_count2VSYNC>, function returns number of loop iterations (L), and 
      // microseconds shall be calculated as ms = 14 + 9*L (CPU Cycles will then be 4*ms)
      // With this, we measure exact time it takes for us to draw the sprite
      ms = 14 + 9 * cpct_count2VSYNC();

      // Print 5 digits on the upper right corner of the screen, 
      // with the amount of free microseconds calculated in previous step. 
      // Digits will be printed at screen locations (0xC046, 0xC048, 0xC04A, 0xC04C, 0xC04E)
      #define LASTDIGIT_VMEM  0xC04E
      for(i=0; i<5; i++) {
         u8 digit = '0' + (ms % 10);
         cpct_drawCharM1((void*)(LASTDIGIT_VMEM - 2*i), digit);
         ms /= 10;
      }
   }
}
