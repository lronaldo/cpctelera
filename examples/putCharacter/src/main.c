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

#include <cpctelera_all.h>

//
// Macro for fastly cleaning the screen, filling it up with 0's
//
#define CLEAR_SCREEN cpct_memset((char*)0xC000, 0x4000, 0);

unsigned char* incrementedVideoPos(unsigned char* video_pos, unsigned char inc) {
  video_pos += inc;
  if (video_pos >= (unsigned char*)0xC7CF) 
    video_pos = (unsigned char*)0xC000;

  return video_pos;
}

void main(void) {
   unsigned char *video_pos = (unsigned char*)0xC000;
   unsigned char charnum, times, fcolor, bcolor;

   cpct_disableFirmware();

   while(1) {
/*      // Mode 2
      CLEAR_SCREEN
      cpct_setVideoMode(2);

      // Print all characters 8 times
      fcolor=0;
      for(times=0; times < 16; times++) {
         fcolor ^= 0x01;
         for(charnum=1; charnum != 0; charnum++) {
            cpct_drawROMCharM2(video_pos, fcolor, charnum);
            video_pos = incrementedVideoPos(video_pos, 1);
         }
      }
*/
      // Mode 1
      CLEAR_SCREEN
      cpct_setVideoMode(1);
      fcolor = bcolor = 0;

      // Print all characters 12 times
      for(times=0; times < 12; times++) {
         // Colors from 0 to 4
         if (++fcolor > 3) {
            fcolor=0;
            bcolor = (bcolor + 1) & 0x03;
         }
         for(charnum=1; charnum != 0; charnum++) {
            cpct_drawROMCharM1_fast(video_pos, fcolor, bcolor, charnum);
            video_pos = incrementedVideoPos(video_pos, 2);
         }
      }
   }
}
