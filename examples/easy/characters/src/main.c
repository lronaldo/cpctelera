//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//
// This function calculates next video memory location, cycling pointer
// when it exceedes the end of standard screen video memory
//
u8* incrementedVideoPos(u8* pvideomem, u8 inc) {
   // Increment video memory pointer in given 'inc' bytes
   pvideomem += inc;

   // Check if video memory is greater that the last screen location where
   // ... a character can be writen (25 character lines go from 0xC000 to 0xC7D0)
   // If we exceed the range, restore the pointer
   if (pvideomem > (u8*)0xC7D0)
      pvideomem = (u8*)0xC000;

   // Return the new incremented position of video memory pointer
   return pvideomem;
}

//
// Drawing Characters example: MAIN
//
void main(void) {
   u8 *pvideomem = (u8*)0xC000;  // Pointer to the start of video memory, to draw first character there
   u8 charnum;                   // Character that will be drawn 
   u8 times;                     // Counter of how many times to repeat drawing characters
   u8 colours[5] = {0};          // Colours values: 1 colour for mode 2, 2 for mode 1 and another 2 for mode 0

   // Disable firmware to prevent it from restoring our video memory changes 
   // ... and interfering with drawChar functions
   cpct_disableFirmware();

   //
   // Loop forever showing characters on different modes and colours
   //
   while(1) {
      //
      // DRAW SOME CHARACTERS ON MODE 2
      //
      cpct_clearScreen(0);    // Clear Screen filling up with 0's
      cpct_setVideoMode(2);   // Set Mode 2 (640x200, 2 colours)

      // Print the complete set of 256 characters 16 times
      for(times=0; times < 16; times++) {
         // Each time we start printing the set, we change colour from 1 to 0 and 0 to 1
         // We do this using and XOR with 1
         colours[0] ^= 0x01;

         // Draw the complete set of 256 characters (excluding char 0)
         for(charnum=1; charnum != 0; charnum++) {
            
            // Draw next character in current colour
            cpct_drawCharM2(pvideomem, colours[0], charnum);

            // Point to next location on screen to draw 
            // (increment 1 byte, as 1 byte = 8 pixels in mode 2)
            pvideomem = incrementedVideoPos(pvideomem, 1);     
         }
      }

      //
      // DRAW SOME CHARACTERS ON MODE 1
      //
      cpct_clearScreen(0);    // Clear Screen filling up with 0's
      cpct_setVideoMode(1);   // Set Mode 1 (320x200, 4 colours)

      // Print the complete set of 256 characters 16 times
      for(times=0; times < 16; times++) {
         // Each time we start printing the set, we change foreground and background
         // colours. Foreground and background colours rotate around the 4 values available
         if (++colours[1] > 3) {
            // Each time all 4 colours have been used for foreground, we increment
            // background and reset foreground to 0.
            colours[1] = 0;
            if (++colours[2] > 3)
               colours[2] = 0;
         }

         // Draw the complete set of 256 characters (excluding char 0)
         for(charnum=1; charnum != 0; charnum++) {
            
            // Draw next character in current foreground and background colours
            cpct_drawCharM1_f(pvideomem, colours[1], colours[2], charnum); 

            // Point to next location on screen to draw 
            // (increment 2 bytes, as 1 byte = 4 pixels in mode 1)
            pvideomem = incrementedVideoPos(pvideomem, 2);
         }  
      }

      //
      // DRAW SOME CHARACTERS ON MODE 0
      //
      cpct_clearScreen(0);    // Clear Screen filling up with 0's
      cpct_setVideoMode(0);   // Set Mode 1 (160x200, 16 colours)


      // Print the complete set of 256 characters 32 times
      // As there are much more colours to show in mode 0
      for(times=0; times < 32; times++) {
         // Each time we start printing the set, we change foreground and background
         // colours. Foreground and background colours rotate around the 16 values available
         if (++colours[3] > 15) {
            // Each time all 16 colours have been used for foreground, we increment
            // background and reset foreground to 0.           
            colours[3] = 0;
            if (++colours[4] > 15)
               colours[4] = 0;
         }

         // Draw the complete set of 256 characters (excluding char 0)
         for(charnum=1; charnum != 0; charnum++) {

            // Draw next character in current foreground and background colours
            cpct_drawCharM0(pvideomem, colours[3], colours[4], charnum);

            // Point to next location on screen to draw 
            // (increment 4 bytes, as 1 byte = 2 pixels in mode 0)
            pvideomem = incrementedVideoPos(pvideomem, 4);
         }
      }
   }
}
