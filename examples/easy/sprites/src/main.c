//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 Stefano Beltran / ByteRealms (stefanobb at gmail dot com)
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

// Number of loops to wait between each sprite drawing
#define WAITLOOPS 150000

// Sizes of the sprites (in bytes)
//  Logo         = (160x191 pixels in Mode 1 => 40x191 bytes)
//  Banner parts = ( 80x96  pixels in Mode 0 => 40x96  bytes)
#define LOGO_W      40
#define LOGO_H     191
#define BANNER_W    40
#define BANNER_H    96

//
// Draw CPCtelera's Squared Banner (Mode 0)
//
void drawBanner() {
    // Video memory pointers for the 2 sprites that form the Squared banner
    u8 *pvideo_s1, *pvideo_s2;

    // Clear the screen filling it up with 0's
    cpct_clearScreen_f64(0);

    // Set Mode 0 Squared Banner palette and change to Mode 0
    cpct_setPalette  (G_banner_palette, 16);
    cpct_setVideoMode(0);

    // Draw CPCtelera's Squared Banner in 2 parts of 80x96 pixels (40x96 bytes in mode 0)
    // We have to draw it in two parts because cpct_drawSprite function cannot 
    // draw sprites wider than 63 bytes. 
    // Remember: in Mode 0, 1 byte = 2 pixels

    // Draw left part at screen byte coordinates  ( 0, 52) (pixel coordinates ( 0, 52))
    pvideo_s1 = cpct_getScreenPtr(CPCT_VMEM_START,  0, 52);
    cpct_drawSprite(G_CPCt_left,  pvideo_s1, BANNER_W, BANNER_H);

    // Draw right part at screen byte coordinates (40, 52) (pixel coordinates (80, 52))
    pvideo_s2 = cpct_getScreenPtr(CPCT_VMEM_START, 40, 52);
    cpct_drawSprite(G_CPCt_right, pvideo_s2, BANNER_W, BANNER_H);
}

//
// Draw CPCtelera's Logo (Mode 1)
//
void drawLogo() {
    // Pointer to video memory location where the logo will be drawn
    u8* pvideo;

    // Clear the screen filling it up with 0's
    cpct_clearScreen_f64(0);

    // Set Mode 1 Logo palette and change to Mode 1
    cpct_setPalette(G_logo_palette, 4);
    cpct_setVideoMode(1);     

    // Draw CPCtelera's Logo as one unique sprite 160x191 pixels (40x191 bytes in mode 1)
    // Remember: in Mode 1, 1 byte = 4 pixels    

    // Draw the sprite at screen byte coordinates (20, 4) (pixel coordinates (80, 4))
    pvideo = cpct_getScreenPtr(CPCT_VMEM_START, 20, 4);
    cpct_drawSprite(G_CPCt_logo, pvideo, LOGO_W, LOGO_H);
}

//
// MAIN: Keyboard check example
//
void main(void) {
    // 32 bits counter, to let it count passed 65536 (up to 4 Billions)
    u32 i;
    
    // Disable firmware to prevent it from interfering with setVideoMode
    cpct_disableFirmware();

    // Convert palettes from firmware colour values to 
    // hardware colours (which are used by cpct_setPalette)
    cpct_fw2hw(G_banner_palette, 16);
    cpct_fw2hw(G_logo_palette, 4);

    // Set the border white, using colour 0 from G_banner_palette 
    // after converting the colours to hardware values
    cpct_setBorder(G_banner_palette[0]);

    // Infinite main loop
    while (1) {
        // Draw CPCtelera's Logo and wait for a while
        drawLogo();
        for(i=0; i < WAITLOOPS; ++i);

        // Draw CPCtelera's Banner and wait for a while
        drawBanner();
        for(i=0; i < WAITLOOPS; ++i);
    }
}
