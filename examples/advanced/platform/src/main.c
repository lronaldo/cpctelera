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
   u8* pscr;   // Pointer to the screen location where we want to draw
   u8  c;      // Colour pattern to draw

   // Disable firmware: we dont want it to interfere with our code
   cpct_disableFirmware();

   // Set the hardware palette (convert firmware colour values to hardware ones and set the palette)
   cpct_fw2hw(G_palette, 16);
   cpct_setPalette(G_palette, 16);
   cpct_setBorder(G_palette[8]);

   // Change to Mode 0 (160x200, 16 colours)
   cpct_setVideoMode(0);

   // Draw limits
   c = cpct_px2byteM0(8,8);
   pscr = cpct_getScreenPtr(g_SCR_VMEM, 54,   0);
   cpct_drawSolidBox(g_SCR_VMEM, c,  4, 200);
   cpct_drawSolidBox(pscr      , c, 26, 200);
   pscr = cpct_getScreenPtr(g_SCR_VMEM,  4, 192);
   cpct_drawSolidBox(g_SCR_VMEM+4, c, 50, 8);
   c = cpct_px2byteM0(6,6);
   cpct_drawSolidBox(pscr        , c, 50, 8);
}

//
// Scan Keyboard and do user actions as requested
//
void updateUser(TCharacter* user) {
   // Scan Keyboard
   cpct_scanKeyboard_f();

   // Check possible keys to press, and do actions
   if ( cpct_isKeyPressed(Key_CursorUp) && user->status != es_jump ) {
      performAction(user, es_jump, user->side);
      g_nextRand += user->entity.nx;    // Randomly move the random seed
   }
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
//#define TESTCPU
void main(void) {
   TCharacter* c;

   // Initialize game
   initializeCPC();
   initializeEntities();
      
   c = getCharacter();

   // Main Game Loop
   while(1) {
      updateUser(c);
      scrollWorld();
      updateCharacter(c);
      cpct_disableFirmware();

#ifdef TESTCPU
      {
         u8 str[10];
         sprintf(str, "%u   ", 22 + 34 * cpct_count2VSYNC());
         cpct_drawStringM0(str, (u8*)0xC000, 1, 0);
      }
#endif

      cpct_waitVSYNC();
      drawAll();
   }
}
