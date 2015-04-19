//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
// Macro for fastly cleaning the screen, filling it up with 0's
//
#define CLEAR_SCREEN cpct_memset((char*)0xC000, 0x4000, 0);

//
// Wait n complete screen frames (1/50)s
//
void wait_frames (unsigned int nframes) {
   unsigned int i;
   for (i=0; i < nframes; i++) 
      cpct_waitVSYNC();
}

void main(void) {
   unsigned char *video_pos, times, colors[5];

   cpct_memset((char*)colors, 5, 0);
   cpct_disableFirmware();

   // Loop forever showing characters on different modes and colors
   //
   while(1) {
      // Print string on mode 0 and wait
      CLEAR_SCREEN;
      cpct_setVideoMode(0);
      video_pos = (unsigned char*)0xC000;
      for (times=0; times < 24; times++) {
         colors[0] = ++colors[0] & 15;
         cpct_drawROMStringM0("$ Mode 0 string $", video_pos, colors[0], colors[3]);
         wait_frames(25);
         video_pos += 0x50;
      }
      colors[3] = ++colors[3] & 15;

      // Print string on mode 1 and wait
      CLEAR_SCREEN;
      cpct_setVideoMode(1);
      video_pos = (unsigned char*)0xC000;
      for (times=0; times < 24; times++) {
         colors[1] = ++colors[1] & 3;
         cpct_drawROMStringM1("Mode 1 string :D", video_pos, colors[1], colors[4]);
         colors[1] = ++colors[1] & 3;
         cpct_drawROMStringM1_fast("Mode 1 string (Fast)", video_pos + 38, colors[1], colors[4]);
         colors[1] = ++colors[1] & 3;
         wait_frames(25);
         video_pos += 0x50;
      }
      colors[4] = ++colors[4] & 3;

      // Print string on mode 2 and wait
      CLEAR_SCREEN;
      cpct_setVideoMode(2);
      video_pos = (unsigned char*)0xC000;
      for (times=0; times < 24; times++) {
         colors[2] ^= 1;
         cpct_drawROMStringM2("And, finally, this is a long mode 2 string!!", video_pos, colors[2]);
         wait_frames(25);
         video_pos += 0x50;
      }
   }
}
