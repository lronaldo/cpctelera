//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

// Size of the sprite (in bytes)
//  Logo         = (160x191 pixels in Mode 1 => 40x191 bytes)
#define LOGO_W      40
#define LOGO_H     191

//
// Pre-calculated sinus table.
//     Sinus wave, expanded by a factor of 20 and moved from [-10,10] to [0, 20]
//     Used to move the logo using a sinusoidal movement
// 
const u8 sinus_offsets[256]={
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,
    8,8,8,8,9,9,9,9,10,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,14,15,15,15,15,16,16,16,16,
    16,17,17,17,17,17,17,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,
    20,20,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19,19,19,19,19,19,19,19,18,18,18,18,18,18,18,17,17,17,17,
    17,17,16,16,16,16,16,15,15,15,15,14,14,14,14,14,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,10,9,9,9,
    9,8,8,8,8,7,7,7,7,6,6,6,6,6,5,5,5,5,4,4,4,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0
};

//
// Draw CPCtelera's Logo (Mode 1)
//
void drawLogo() {
    // Pointer to video memory location where the logo will be drawn
    u8* pvideo;

    // Clear the screen filling it up with 0's
    cpct_clearScreen(0);

    // Initialize palette and video mode    
    cpct_fw2hw(G_logo_palette, 4);      // Convert palettes from firmware colour values to hardware colours
    cpct_setPalette(G_logo_palette, 4); // Set hardware palette   
    cpct_setBorder(G_logo_palette[0]);  // Set the border white, using colour 0 from palette (after converting it to hardware values)
    cpct_setVideoMode(1);               // Set video mode 1 (320x200 pixels)

    // Draw CPCtelera's Logo as one unique sprite 160x191 pixels (40x191 bytes in mode 1)
    // Remember: in Mode 1, 1 byte = 4 pixels    

    // Draw the sprite at screen byte coordinates (40, 4) (pixel coordinates (160, 4))
    // The sprite will be at its rightmost position to be able to use left scrolling 
    // to control it. Left scrolling at this position only requires to move the Video Screen
    // pointer forward, which makes pixels "move to the left" as start of video memory becomes
    // nearer to them.
    pvideo = cpct_getScreenPtr(CPCT_VMEM_START, 40, 4);
    cpct_drawSprite(G_CPCt_logo, pvideo, LOGO_W, LOGO_H);
}

//
// MAIN: Basic hardware scroll example
//
void main(void) { 
    u8 i=0; // Iterations counter: loops through the sinus_offsets table.
    
    // Initialize screen and draw logo
    cpct_disableFirmware(); // Disable firmware to prevent it from interfering with setVideoMode
    drawLogo();             // Initialize palette and draw CPCtelera's Logo 
        
    //
    // Main Loop: infinitely move cup side-to-side
    //
    while (1) {
        // Move the screen video pointer to a new offset (4-by-4 bytes), causing an horizontal scroll
        // We move the pointer forward up to 4*20 bytes = 80 bytes (1 complete pixel line) and then
        // return it to original offset. This makes pixels move to the left up to 1 line and 
        // return to their original position
        cpct_setVideoMemoryOffset(sinus_offsets[i++]);  
        
        // Synchronize with VSYNC + 1 HSYNC to slow down the movement
        cpct_waitVSYNC();   // Wait for VSYNC signal
        __asm__("halt");    // HALT assembler instruction makes CPU wait till next HSYNC signal
    }
}
