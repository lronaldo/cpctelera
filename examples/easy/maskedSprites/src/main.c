//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "sprites.def"

char* videoMem (unsigned char x, unsigned char y) {
   const char* videoMemoryStart = (const char*)0xC000;
   return (char *)(videoMemoryStart + (y >> 3) * 0x50 + (y & 7) * 0x800 + (x >> 1));
}

void main(void) {
   unsigned char x=0, y=0;
   char *charlies[8] = { videoMem(  0,   0), videoMem( 32,  40),
                         videoMem(100,  16), videoMem(132, 100),
                         videoMem( 64, 160), videoMem(150,  80),
                         videoMem( 48,  20), videoMem( 80,   8) };

   cpct_disableFirmware();
   cpct_setVideoMode(0);

   // Make a background full of Newtons
   for (x=0; x < 5; x++) {
      for(y=0; y < 6; y++) {
         cpct_drawSprite(G_newton_sprite, videoMem(x << 5, y << 5), 16, 32);
      }
   }

   // Paint an array of charlies
   for (x=0; x < 8; x++) 
      cpct_drawSpriteMasked(G_charlie_sprite, charlies[x], 4, 16);

   while(1) {}
}
