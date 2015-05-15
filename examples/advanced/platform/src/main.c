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

#include <stdio.h>
#include <cpctelera.h>
#include "entities/entities.h"
#include "entities/sprites.h"

//
// Initialization of the Amstrad CPC at the start of the game
//   Sets Palette and Mode, and disables firmware
//
void initializeCPC() {
   // Disable firmware: we dont want it to interfere with our code
   cpct_disableFirmware();

   // Set the hardware palette (convert firmware colour values to hardware ones and set the palette)
   cpct_fw2hw(G_palette, 16);
   cpct_setPalette(G_palette, 16);

   // Change to Mode 0 (160x200, 16 colours)
   cpct_setVideoMode(0);
}

//
// Scan Keyboard and do user actions as requested
//
void updateUser(TCharacter* user) {
   // Scan Keyboard
   cpct_scanKeyboard_f();

   // Check possible keys to press, and do actions
   if ( cpct_isKeyPressed(Key_CursorUp) )
      performAction(user, es_jump, user->side);
   else if ( cpct_isKeyPressed(Key_CursorRight) )
      performAction(user, es_walk, s_right);
   else if ( cpct_isKeyPressed(Key_CursorLeft)  ) 
      performAction(user, es_walk, s_left); 
   else
      performAction(user, es_static, user->side);
}

//////////////////////////////////////////////////////////////////////
// MAIN EXAMPLE CODE
//    Keys:
//
void main(void) {
   TCharacter* c;

   // Initialize game
   initializeCPC();
   initializeEntities();
   newSolidBlock(20, 100, 10, 3, 0xA0);
   c = getCharacter();

   // Main Game Loop
   while(1) {
      updateUser(c);
      updateCharacter(c);
      cpct_disableFirmware();
      cpct_waitVSYNC();
      drawAll();
   }
}
