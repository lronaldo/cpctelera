//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
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
#include <stdio.h>

// Program Stack locations
#define NEW_STACK_LOCATION       (void*)0x200
#define PREVIOUS_STACK_LOCATION  (void*)0xC000

//
// Print some messages using firmware
//
void printMessages() {
   u8 i;

   // Print some messages using printf (with firmware)
   printf("     \017\003SET PROGRAM STACK LOCATION DEMO\n\r");
   printf("    \017\002#################################\n\n\r");

   printf("\017\001  This program  changes  stack  location");
   printf("to \017\0030x200\017\001, just  below  the start  of the");
   printf("main function.\n\n\r");

   printf("  With this change, the 3rd  memory bank");
   printf("can be  enterely used as  double buffer,");
   printf("making easier to map code and  data into");
   printf("memory.\n\n\r");

   printf("  If you  want to  check this, open  the");
   printf("debugger and  have  a look at the  stack");
   printf("pointer (\017\002SP\017\001) and the stack contents.\n\n\r");
 
   printf("  Now you can use \017\002keys \017\003[\017\0021\017\003]");
   printf("\017\001&\017\003[\017\0022\017\003]\017\001 to change");
   printf("from main  screen  buffer to  the double");
   printf("buffer which contains a fullscreen image");
   printf("with a pattern like this:\n\n\r");

   for (i=0; i < 60; ++i)
      printf("\017\001#\017\003#");
}

//
// MAIN PROGRAM
//
void main(void) { 
   // Clear the screen with 0's and print some messages using firmware
   cpct_clearScreen_f64(0x0000);
   printMessages();

   // Firmware should be disabled when changing stack location,
   // as it may restore original stack location, causing unexpected behaviour
   cpct_disableFirmware();

   // Set the stack to its new location. As this program starts at 0x200, 
   // we move the stack to 0x1FF. The stack will grow from there to 0x00
   // When doing this call, stack has 6 bytes stored in it. We copy them
   // previous to stack change (Beware! This contents may change if main 
   // function changes!)
   cpct_memcpy(NEW_STACK_LOCATION - 6, PREVIOUS_STACK_LOCATION - 6, 6);
   cpct_setStackLocation(NEW_STACK_LOCATION - 6);

   // Clear backbuffer at 0x8000, that can be used
   // now because the stack has been moved to a new location
   cpct_memset_f64((void*)0x8000, 0xFFF0, 0x4000);

   // Infinite Loop
   //   Read Keyboard and change screen video memory page on demand
   while(1) {
      cpct_waitVSYNC();
      cpct_scanKeyboard();     

      if (cpct_isKeyPressed(Key_1)) {
         cpct_setBorder(4);
         cpct_setVideoMemoryPage(cpct_pageC0);
      } else if (cpct_isKeyPressed(Key_2)) {
         cpct_setBorder(3);
         cpct_setVideoMemoryPage(cpct_page80);
      }
   }
}
