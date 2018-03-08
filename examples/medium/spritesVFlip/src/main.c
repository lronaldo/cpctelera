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
#include <rocket.h>     // Generated header in src/ folder after img/rocket.png automatic conversion

///////////////////////////////////////////////////////////////////
// DEFINE CONSTANTS
//
#define INIT_X 20    // Initial coordinates for sprites
#define INIT_Y 100   //

///////////////////////////////////////////////////////////////////
// DRAW ROCKETS
//    Draws a rocket twice at location x, y in the screen. The
// left rocket is drawn normal, and the right one is drawn 1 byte
// to the right (2 mode 0 pixels) and vertically flipped.
//
void drawRockets(u8 x, u8 y) {
   u8* pvmem;  // Pointer to video memory

   //-----Draw the left sprite
   //
   // Get a pointer to video memory byte for location (x, y)
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
   // Draw g_rocket sprite at location (x, y) pointed by pvmem
   cpct_drawSprite(g_rocket, pvmem, G_ROCKET_W, G_ROCKET_H);

   //-----Draw the Right sprite
   //
   // Assuming pvmem points to upper-left byte of the rocket sprite in
   // video memory, calculate a pointer to the bottom-left byte.
   // Equivalent to: cpct_getScreenPtr(CPCT_VMEM_START, x, (y + G_ROCKET_H - 1) )
   pvmem = cpct_getBottomLeftPtr(pvmem, G_ROCKET_H);
   // As we don't want to overwrite the left rocket, this right rocket will
   // be drawn 1 byte to its right. That means moving to the right (adding) 
   // a number of bytes equal to the width of the rocket + 1. 
   pvmem += G_ROCKET_W + 1;
   // Finally, draw the right rocket vertically flipped. This draw function
   // does the drawing bottom-to-top in the video memory. That's the reason
   // to have a pointer to the bottom-left.
   cpct_drawSpriteVFlip(g_rocket, pvmem, G_ROCKET_W, G_ROCKET_H);  

   // cpct_drawSpriteVFlip_f could be used instead for faster drawing, 
   // at the cost of more memory consumption (+125 bytes, as it uses an unrolled loop)
}

///////////////////////////////////////////////////////////////////
// CLEAR ROCKETS
//    Draws a black solid box to clear both rockets at once.
//
void clearRockets(u8 x, u8 y) {
   u8* pvmem;  // Pointer to video memory

   //-----Overwrite sprites with a Black Solid Box
   //
   // Get a pointer to video memory byte location for (x, y)
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
   // Draw a Black Solid Box (color index 0 for both pixels of each byte).
   // Width of the box must be 2 times the width of a Rocket + 1 (as both
   // rockets are separated by 1 byte). Height is the same as the rockets.
   cpct_drawSolidBox(pvmem, cpctm_px2byteM0(0, 0), 2*G_ROCKET_W+1, G_ROCKET_H);
}

///////////////////////////////////////////////////////////////////
// GET USER INPUT
//    Checks user input (keyboard) and sets movement velocity
// (vx, vy). Velocities are passed by reference.
//
void getUserInput(i8* vx, i8* vy) {
   // Velocities will be 0 unless a keypress is detected
   *vx = *vy = 0;

   // Scan the keyboard and update keypresses
   cpct_scanKeyboard_f();

   // Check for user keys (OPQA) one by one, and update velocities
   // by one byte depending on which ones are pressed
   if ( cpct_isKeyPressed(Key_O) ) (*vx)--;  // O: Left
   if ( cpct_isKeyPressed(Key_P) ) (*vx)++;  // P: Right
   if ( cpct_isKeyPressed(Key_Q) ) (*vy)--;  // Q: Up
   if ( cpct_isKeyPressed(Key_A) ) (*vy)++;  // A: Down
}

///////////////////////////////////////////////////////////////////
// INITIALIZATION
//    Sets the initial configuration for the CPC
//
void initialize() {
   cpct_disableFirmware();          // Disable firmware to prevent it from restoring mode and palette
   cpct_setVideoMode(0);            // Set video mode to 0 (160x200, 16 colours)
   cpct_setPalette(g_palette, 16);  // Set the palette using hardware values generated at rocket.h
   cpct_setBorder(HW_BLACK);        // Set border colour to Black 
}

///////////////////////////////////////////////////////////////////
// MAIN PROGRAM
//
void main(void) {
   u8 x = INIT_X, y = INIT_Y;    // x, y coordinates
   i8 vx = 1, vy = 0;            // vx, vy velocity (vx not 0 to force initial drawing)

   // First of all, initialize the CPC
   initialize();

   // Perform the Main Loop forever
   while (1) {
      // Wait to the VSYNC signal to ensure that any drawing we 
      // do is performed afterwards to prevent raster from overtaking us
      // and generating flickering. Also, limiting action to 50Hz
      cpct_waitVSYNC();
      
      // Only perform any new drawing to screen whenever there 
      // is going to be any movement (vx or vy not 0)
      //    BEWARE! Rockets may move outside screen boundaries, because 
      //    (x, y) coordinates are updated without any boundaries check.
      if (vx || vy) {
         clearRockets(x, y);  // Clear Rockets
         x += vx; y += vy;    // Update x,y coordinates according to velocity
         drawRockets(x, y);   // Draw Rockets at their new location
      }

      // Get user input for next movements
      getUserInput(&vx, &vy);
   }
}
