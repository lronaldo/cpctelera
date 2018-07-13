//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//////////////////////////////////////////////////////////////////////////////////////////////
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
      pvideomem = (u8*)CPCT_VMEM_START;

   // Return the new incremented position of video memory pointer
   return pvideomem;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Print all 256 characters using a concrete drawing function and increment
//
void print256Chars(u8 **pvideomem, u8 mode, u8 fg_colour, u8 bg_colour) {
   const u8 increments[3] = { 4, 2, 1 };
   u8 charnum, increment;

   // Calculate increment in bytes, for every time we want to move 
   // video memory pointer to the next character
   increment = increments[mode];

   // Set colours to be used for text drawing
   switch (mode) {
      case 2: cpct_setDrawCharM2(fg_colour, bg_colour);  break;
      case 1: cpct_setDrawCharM1(fg_colour, bg_colour);  break;
      case 0: cpct_setDrawCharM0(fg_colour, bg_colour);  break;
   }

   // Draw the complete set of 256 characters (excluding char 0)
   for(charnum=1; charnum != 0; charnum++) {
      switch (mode) {
         case 2: cpct_drawCharM2  (*pvideomem, charnum); break;
         case 1: cpct_drawCharM1  (*pvideomem, charnum); break;
         case 0: cpct_drawCharM0  (*pvideomem, charnum); break;
      }
      // Point to next location on screen to draw (increment bytes required for this mode)
      *pvideomem = incrementedVideoPos(*pvideomem, increment);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function to repeat N times drawing 256 characters on a given mode
//    The function modifies video memory pointer and colours (received by reference)
//
void drawCharacters(u8** pvideomem, u8 maxtimes, u8 mode, u8* fg_colour, u8* bg_colour) {
   u8 times;                            // Loop counter for times to repeat
   const u8 colours[3] = { 16, 4, 2 };  // Number of colour each mode has (0 = 16, 1 = 4, 2 = 2)

   cpct_clearScreen(0);     // Clear Screen filling up with 0's
   cpct_setVideoMode(mode); // Set desired video mode

   // Print the complete set of 256 characters maxtimes times
   for(times=0; times < maxtimes; times++) {
      // Each time we start printing the set, we change foreground and background colours.
      // We loop foreground colours up to the max colour, and then loop background colours
      if (++(*fg_colour) == colours[mode]) {
         *fg_colour = 0;
         if (++(*bg_colour) == colours[mode])
            *bg_colour = 0;
      }
      
      // Print all 256 chars in mode 1 usingcurrent colours
      print256Chars(pvideomem, mode, *fg_colour, *bg_colour);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Drawing Characters example: MAIN
//
void main(void) {
   u8* pvideomem  = CPCT_VMEM_START; // Pointer to video memory
   u8  colours[6] = {0};             // 6 variables for 3 pairs of foreground / background colour

   // Disable firmware to prevent it from restoring our video memory changes 
   // ... and interfering with drawChar functions
   cpct_disableFirmware();
   
   // Loop forever showing characters on different modes and colours
   //
   while(1) {
      drawCharacters(&pvideomem, 14, 2, (colours+0), (colours+1)); // Drawing on mode 2, 14 times
      drawCharacters(&pvideomem, 17, 1, (colours+2), (colours+3)); // Drawing on mode 1, 17 times
      drawCharacters(&pvideomem, 21, 0, (colours+4), (colours+5)); // Drawing on mode 0, 21 times
   }
}
