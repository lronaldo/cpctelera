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
#include "sprite_definitions.inc"

//
// Macro for fastly cleaning the screen, filling it up with 0's
//
#define CLEAR_SCREEN cpct_memset((char*)0xC000, 0, 0x4000);

//
// Convenient type definitions
//
typedef unsigned char Byte;
// 4 names for our 4 types of drawSprite functions
typedef enum { _2x8, _4x8, _2x8Fast, _4x8Fast } TDrawFunc;

// Constants used to control length of waiting time
const unsigned int WAITCLEARED = 20000; 
const unsigned int WAITPAINTED = 60000; 

// Tables with values for each of the 4 iterations of the main loop
//  (sprite to use, width of the sprite and draw function to use)
const Byte* const  sprites[4] = { waves_2x8,     F_2x8, waves_4x8,    FF_4x8 };
const Byte      spr_widths[4] = {         2,         2,         4,         4 };
const TDrawFunc  functions[4] = {      _2x8,  _2x8Fast,      _4x8,  _4x8Fast };

//
// Fills all the screen with sprites using drawSprite_XXX_aligned functions
//
void printAllScreen(Byte* sprite, Byte spritewidth, TDrawFunc function) {
   Byte *video_mem = (Byte*)0xC000, x, y;

   // Cover all the screen (25 lines) with tiles
   for (y=0; y < 25; y++) { 
      for (x=0; x < (80/spritewidth); x++) {
         switch (function) {
            case _2x8Fast: cpct_drawSpriteAligned2x8_f(sprite, video_mem); break;
            case _2x8:     cpct_drawSpriteAligned2x8  (sprite, video_mem); break;
            case _4x8Fast: cpct_drawSpriteAligned4x8_f(sprite, video_mem); break;
            case _4x8:     cpct_drawSpriteAligned4x8  (sprite, video_mem); break;
         }
         video_mem += spritewidth;
      }
   }
}

void main(void) {

   // Initialization
   cpct_disableFirmware();
   cpct_setVideoMode(0);

   // Main loop: filling the screen using the 4 different basic 
   //            aligned functions in turns.
   while(1) {
      Byte i;
      unsigned int w;

      for (i=0; i < 4; i++) {
         CLEAR_SCREEN
         for (w=0; w < WAITCLEARED; w++);
         printAllScreen(sprites[i], spr_widths[i], functions[i]);
         for (w=0; w < WAITPAINTED; w++);
      }
   }
}
