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
#include "runner.h"
#include "banner.h"

// Sprite size (in bytes)
#define SP_W   10
#define SP_H   94
#define SP_X   34
#define SP_Y   86

// Banner size (in bytes)
#define BANNER_W  80
#define BANNER_H  52

// Size of the screen (in bytes)
#define SCR_W  80

// Floor location and colour pattern
#define FLOOR_Y      180
#define FLOOR_X       30 
#define FLOOR_W       20
#define FLOOR_H       10

// Animation and timing
#define ANIM_FRAMES     6
u8* const g_animation[6] = {
   g_runner_0, g_runner_1, g_runner_2,
   g_runner_3, g_runner_4, g_runner_5
};

//
// INITIALIZE: Initialize CPC, Draw demo banner and instructions
//
void initialize() {
   u8* pvideomem;               // Pointer to video memory

   // Disable firmware to prevent it from interfering with setPalette and setVideoMode
   cpct_disableFirmware();

   // Set up the hardware palette using hardware colour values
   cpct_setBorder(HW_BLACK);
   cpct_setPALColour(0, HW_BLACK);
   cpct_setPALColour(1, HW_WHITE);
   
   // Set video mode 2 (640x200, 2 colours)
   cpct_setVideoMode(2);

   // Draw Demo banner at top-left corner of the screen (Start of video memory).
   // We draw it in 2 parts, as cpct_drawSprite cannot draw sprites wider than 63 bytes.
   cpct_drawSprite(g_banner_0, CPCT_VMEM_START             , BANNER_W/2, BANNER_H);
   cpct_drawSprite(g_banner_1, CPCT_VMEM_START + BANNER_W/2, BANNER_W/2, BANNER_H);

   // Draw instructions
   pvideomem = cpct_getScreenPtr(CPCT_VMEM_START, 29, 60);
   cpct_setDrawCharM2(0, 1);
   cpct_drawStringM2("[Any Key] Run Opposite", pvideomem);
}

//
// MAIN: Using keyboard to move a sprite example
//
void main(void) {
   u8  frame  = 0;  // Actual animation frame
   u8  cycles = 0;  // Number of waiting cycles done for present frame
   u8* pvmem_spr;   // Video memory location where sprites will be drawn
   u8* pvmem_floor; // Video memory location where the floor will be drawn
   u8  floor_color = 0b1101010; // Pixel pattern for the floor
   u8  wait_cycles = 6; // Number of cycles to wait for each animation frame

   // Initialize the Amstrad CPC
   initialize();

   // Sprites and the floor are always drawn at the same place. 
   // We only need to calculate them once
   pvmem_spr   = cpct_getScreenPtr(CPCT_VMEM_START, SP_X, SP_Y);
   pvmem_floor = cpct_getScreenPtr(CPCT_VMEM_START, FLOOR_X, FLOOR_Y);

   // Infinite animation loop
   //
   while(1) {
      // Scan Keyboard (fastest routine)
      // The Keyboard has to be scanned to obtain pressed / not pressed status of
      // every key before checking each individual key's status.
      cpct_scanKeyboard_f();

      // If any key is pressed, invert animation
      if (cpct_isAnyKeyPressed()) {
         u8 i = ANIM_FRAMES;
         while (i--) {
            cpct_hflipSpriteM2(SP_W, SP_H, g_animation[i]);
            //// This operation can also be done using ROM-friendly version code
            //cpct_hflipSpriteM2_r(g_animation[i], SP_W, SP_H);
         }
      }
 
      // Check if we have to advance to the next animation frame
      // And advance, if it is the case.
      if (++cycles == wait_cycles) {
         cycles = 0;                   // Restart frame counter
         if (++frame == ANIM_FRAMES) {  // Next animation frame
            frame = 0;
         }
            // Advance floor 
            floor_color ^= 0xFF;
      }

      // Wait for VSYNC and then draw the sprite
      cpct_waitVSYNC();

      // Draw the sprite in the video memory location got from coordinates x, y
      cpct_drawSprite(g_animation[frame], pvmem_spr, SP_W, SP_H);

      // Draw floor (moving the pi)
      cpct_drawSolidBox(pvmem_floor, floor_color, FLOOR_W, FLOOR_H);
   }
}
