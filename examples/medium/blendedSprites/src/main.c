//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "img/scifi_bg.h"

#define SCR_VMEM  (u8*)0xC000

// Firmware palette values
#define G_PALETTE_SIZE 6
const u8 g_palette[G_PALETTE_SIZE] = { 
    HW_BLACK      , HW_BLUE  , HW_RED
   ,HW_BRIGHT_RED , HW_WHITE , HW_PASTEL_BLUE
};

/////////////////////////////////////////////////////////////////////////
// drawBackground
//    Draws the background from pixel line 72 onwards
//
void drawBackground() {
   u8* p = cpct_getScreenPtr(SCR_VMEM, 0, 72);
   cpct_drawSprite(g_scifi_bg_0, p   , 40, 128);
   cpct_drawSprite(g_scifi_bg_1, p+40, 40, 128);
}

/////////////////////////////////////////////////////////////////////////
// Initialization routine
//    Disables firmware, initializes palette and video mode
//
void initialize (){ 
   // Disable firmware to prevent it from interfering
   cpct_disableFirmware();
   
   // 1. Set the palette colours using hardware colour values
   // 2. Set border colour to black
   // 3. Set video mode to 0 (160x200, 16 colours)
   cpct_setPalette(g_palette, G_PALETTE_SIZE);
   cpct_setBorder (HW_BLACK);
   cpct_setVideoMode(0);
}

/////////////////////////////////////////////////////////////////////////
// Main entry point of the application
//
void main(void) {
   initialize();  // Initialize screen, palette and background
   drawBackground();

   while(1);
}
