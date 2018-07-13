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

//
// This example shows how to mix LCG and GLFSR random number generators
// to create a better random number generator.
// It also shows how to do a visual test of the random occurrence of 
// the numbers generated
//
void initializeRandomGenerators() {
   cpct_setSeed_lcg_u8 (0x55);
   cpct_setSeed_glfsr16(0x1120);
   cpct_setTaps_glfsr16(GLFSR16_TAPSET_0512);
}

u8 mixedRandomGenerator() {
   return cpct_getRandom_lcg_u8( cpct_getRandom_glfsr16_u8() );
}

void putpixel(u16 x, u8 y, u8 val) {
   u8* vram = cpct_getScreenPtr(CPCT_VMEM_START, x >> 3, y + 8);
   u8  byte = *vram;
   u8  pen  = val << (7 - (x & 7));
   *vram = byte & (pen ^ 0xFF) | pen;
}

void drawPixelCount(u16 pixels) {
   if (! (pixels & 15) ) {
      char str[7];
      sprintf(str, "%6u", pixels);
      cpct_drawStringM2(str, CPCT_VMEM_START+26);
  }
}

void main(void) {
   u8 last_y = 0x20;
   u32     i = 0;
   
   cpct_disableFirmware();
   cpct_setVideoMode(2);
   cpct_setDrawCharM2(1, 0);
   initializeRandomGenerators();

   cpct_drawStringM2("Random numbers generated:", CPCT_VMEM_START);
   while(1) {
      u8 y = mixedRandomGenerator();
      putpixel(last_y, y>>1, 1);
      drawPixelCount(i++);
      last_y = y;
   }
}
