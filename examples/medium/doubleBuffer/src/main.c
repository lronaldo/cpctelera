//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 Stefano Beltran / ByteRealms (stefanobb at gmail dot com)
//  Copyright (C) 2015 Maximo / Cheesetea / ByteRealms (@rgallego87)
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

// Size of each part of the CPCtelera Logo
#define CPCT_W 40
#define CPCT_H 96

// Size of the ByteRealms Logo
#define BR_W   62
#define BR_H   90

// Pointers to the hardware backbuffer, placed in bank 1 
// of the memory (0x4000-0x7FFF)
#define SCR_BUFF  (u8*)0x4000

///////////////////////////////////////////////////////////////////////////////////////
// Change Video Memory Page
//    This function changes what is drawn on screen by changing video memory page. 
// It alternates between page C0 (0xC000 - 0xFFFF) to page 40 (0x4000 - 0x7FFF). 
// Page C0 is default video memory, page 40 is used in this example as Back Buffer.
// It counts cycles (number of times this function is called) and changes from
// one buffer to the other when cycles exceed waitcycles (a parameter given).
//
void changeVideoMemoryPage(u8 waitcycles) {
   static u8 cycles = 0;   // Static value to count the number of times this function has been called
   static u8 page   = 0;   // Static value to remember the last page shown (0 = page 40, 1 = page C0)

   // Count 1 more cycle and check if we have arrived to waitcycles
   if (++cycles >= waitcycles) {     
      cycles = 0;    // We have arrived, restore count to 0
      
      // Depending on which was the last page shown, we show the other 
      // now, and change the page for the next time 
      if (page) {
         cpct_setVideoMemoryPage(cpct_pageC0);  // Set video memory at banck 3 (0xC000 - 0xFFFF)
         page = 0;                              // Next page = 0
      } else {
         cpct_setVideoMemoryPage(cpct_page40);  // Set video memory at banck 1 (0x4000 - 0x7FFF)
         page = 1;                              // Next page = 1
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////
// MAIN: Hardware Double Buffer example.
//    Shows how to draw sprites in different screen video buffers and how to change
// which one is drawn into the screen.
//
void main(void) {
   u8  br_y = 55; // Y coordinate of the ByteRealms Logo 
   i8  vy   = 1;  // Velocity of the ByteRealms Logo in Y axis
   u8* pvmem;     // Pointer to video memory (or backbuffer) where to draw sprites

   // Initialize CPC Mode and Colours
   cpct_disableFirmware();             // Disable firmware to prevent it from interfering
   cpct_fw2hw       (G_palette, 16);   // Convert Firmware colours to Hardware colours 
   cpct_setPalette  (G_palette, 16);   // Set up palette using hardware colours
   cpct_setBorder   (G_palette[0]);    // Set up the border to the background colour (white)
   cpct_setVideoMode(0);               // Change to Mode 0 (160x200, 16 colours)

   // Clean up Screen and BackBuffer filling them up with 0's
   cpct_memset(CPCT_VMEM_START, 0x00, 0x4000);
   cpct_memset(       SCR_BUFF, 0x00, 0x4000);

   // Lets Draw CPCtelera's Squared Logo on the BackBuffer. We draw it at 
   // byte coordinates (0, 52) with respect to the start of the Backbuffer.
   // We have to draw it into 2 parts because drawSprite function cannot draw
   // sprites wider than 63 bytes (and we have to draw 80). So we draw the
   // logo in two 40-bytes wide parts.
   pvmem = cpct_getScreenPtr(SCR_BUFF, 0,   52);
   cpct_drawSprite(G_CPCt_left,  pvmem,          CPCT_W, CPCT_H);
   cpct_drawSprite(G_CPCt_right, pvmem + CPCT_W, CPCT_W, CPCT_H);

   //
   // Inifite loop moving BR Logo and changing buffers regularly
   //
   while(1) {
      // Draw the ByteRealms logo at its current Y location on the screen. Moving
      // the logo does not leave a trail because we move it pixel to pixel and the
      // sprite has a 0x00 frame around it in its pixel definition.
      pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 10, br_y);  // Locate sprite at (10,br_y) in Default Video Memory
      cpct_drawSprite(G_BR, pvmem, BR_W, BR_H);       // Draw the sprite

      // Change video memory page (from Screen Memory to Back Buffer and vice-versa)
      // every 2.5 secs (125 VSYNCs)
      changeVideoMemoryPage(125);

      // Calculate next location of the ByteRealms logo
      br_y += vy;                            // Add current velocity to Y coordinate
      if ( br_y < 1 || br_y + BR_H > 199 )   // Check if it exceeds boundaries
         vy = -vy;                           // When we exceed boundaries, we change velocity sense

      // Synchronize next frame drawing with VSYNC
      cpct_waitVSYNC();
   }
}
