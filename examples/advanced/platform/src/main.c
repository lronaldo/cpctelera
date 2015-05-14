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
void updateUser(TEntity* user) {
   TPhysics *p = &user->phys;

   // Scan Keyboard
   cpct_scanKeyboard_f();

   // Check possible keys to press, and do actions
   if      ( cpct_isKeyPressed(Key_CursorRight) ) p->vx =  SCALE;
   else if ( cpct_isKeyPressed(Key_CursorLeft)  ) p->vx = -SCALE;
   else if ( p->vx ) {
      /*
      if (p->vx > SCALE) 
         p->vx /= 2;
      else
         p->vx = 0;
         */
      p->vx = 0;
   }

   // Set new animation, based on action requested
//   if (animrequest != es_stop)
//      setAnimation(user, animrequest);

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
      updateUser(&c->entity);
      updateCharacter(c);
      cpct_waitVSYNC();
      drawAll();
   }
}
