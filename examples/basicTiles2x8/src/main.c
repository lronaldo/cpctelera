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

const unsigned char sprite[16] = {
        0x00, 0x00,
        0x10, 0x10,
        0x20, 0x20,
        0x03, 0x03,
        0x04, 0x04,
        0x50, 0x50,
        0x60, 0x60,
        0x07, 0x07
    };


void main(void) {
   unsigned char *video_mem = (unsigned char*)0xC000, x, y;

   cpct_disableFirmware();
   cpct_setVideoMode(0);

   // Cover all the screen with 2x8 tiles
   for (y=0; y < 25; y++) { 
      for (x=0; x < 40; x++) {
         cpct_drawSprite2x8_aligned(sprite, video_mem);
         video_mem += 2;
      }
   }

   while(1);
}
