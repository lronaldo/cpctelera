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

void main(void) {
   unsigned char *video_pos = (unsigned char*)0xC000, charnum, color=0;

   cpct_disableFirmware();
   cpct_setVideoMode(2);

   while(1) {
      // Alternate color between 0 and 1 (XOR)
      color ^= 0x01;

      // Print all characters from 32 to 255
      for(charnum=1; charnum != 0; charnum++) {
         cpct_drawROMCharM2(charnum, color, video_pos);

         // Increment video_pos (characters are 1 byte wide, and
         //   there are 80x25 = 2000 (7D0h) total characters on one mode 2 screen)
         if (video_pos == (unsigned char*)0xC7CF) 
            video_pos = (unsigned char*)0xC000;
         else 
            video_pos++;
      }
   }
}
