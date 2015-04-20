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
#include "entities.h"
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
   TEntity* persea;

   initializeCPC();
   persea = getPersea();

   while(1) {
      char spc = 0;
      
      cpct_scanKeyboardFast();
      if (cpct_isKeyPressed(Key_Space) ) spc = 1;
      
      if (cpct_isKeyPressed(Key_CursorRight) ) {
         if (spc) setAnimation(persea, es_kick);
             else setAnimation(persea, es_walk_right);
      } else if (cpct_isKeyPressed(Key_CursorLeft)) { 
         if (spc) setAnimation(persea, es_kick);
             else setAnimation(persea, es_walk_left);         
      } else if (spc) {
         setAnimation(persea, es_fist);
      }
      updateEntity(persea);

      cpct_waitVSYNC();
      drawEntity(persea);      
   }
}
