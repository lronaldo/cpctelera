//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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
#include "sprites.h"

//
// Initialization of the Amstrad CPC at the start of the game
//   Sets Palette and Mode, and disables firmware
//
void initializeCPC() {
   // Disable firmware: we dont want it to interfere with our code
   cpct_disableFirmware();

   // Set the hardware palette (convert firmware colour values to hardware ones and set the palette)
   cpct_fw2hw(gc_palette, 16);
   cpct_setVideoPaletteHW(gc_palette, 16);

   // Change to Mode 0 (160x200, 16 colours)
   cpct_setVideoMode(0);
}

//
// MAIN EXAMPLE CODE
//
void main(void) {
   unsigned char x=0, y=0;
   char* dest = (char*)0xC000;

   initializeCPC();

   while(1) {
      cpct_scanKeyboardFast();
      if      (cpct_isKeyPressed(Key_CursorRight) && x < 64 ) { x++; dest++; }
      else if (cpct_isKeyPressed(Key_CursorLeft)  && x > 0  ) { x--; dest--; }
      if      (cpct_isKeyPressed(Key_CursorUp)    && y > 0  ) { dest -= (y-- & 7) ? 0x0800 : 0xC850; }
      else if (cpct_isKeyPressed(Key_CursorDown)  && y < 168) { dest += (++y & 7) ? 0x0800 : 0xC850; }

      cpct_drawSprite(gc_PerseaWalk13, dest, 8, 24);
   }
}
