//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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
#include "sprites/sprites.h"
#include "game.h"

////////////////////////////////////////////////////////////////////////////////
// Initialization of the Amstrad CPC at the start of the application
//   Sets Palette and Mode, and disables firmware
//
void initializeCPC() {
   // Disable firmware: we dont want it to interfere with our code
   cpct_disableFirmware();

   // Set the hardware palette (convert firmware colour values to hardware ones and set the palette)
   cpct_fw2hw(G_palette, 16);
   cpct_setPalette(G_palette, 16);
   cpct_setBorder(G_palette[8]);

   // Change to Mode 0 (160x200, 16 colours)
   cpct_setVideoMode(0);
}


//////////////////////////////////////////////////////////////////////
// CODE ENTRANCE POINT FOR THIS EXAMPLE
//
void main(void) {
   u16 score;     // Accumulated score in a game
   u16 hi = 0;    // Hi-score

   // Initialize CPC before starting the game
   initializeCPC();

   //
   // Inifinite loop (play game, end, start another game)
   //
   while(1) {
      score = game(hi);    // Play a game and get the score
      showGameEnd(score);  // Show end-game stats
      
      if (score > hi)      // Update hi-score
         hi = score;
   }
}
