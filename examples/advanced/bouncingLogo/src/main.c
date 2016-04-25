//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Logo Fremos, included in this example, is
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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
#include "entities/logofremos.h"
#include "entities/entities.h"
#include "msg/message.h"

//
// Checks user input and returns acceleration values based on
// the keys pressed by the user
//
void checkUserInput (f32 *ax, f32 *ay) {
   // Read the keyboard
   cpct_scanKeyboard_f();

   // Check for pressed keys and do correspondant actions
   // Cursor keys = apply acceleration to the entity
   if      (cpct_isKeyPressed(Key_CursorRight) ) { *ax=0.5;  }
   else if (cpct_isKeyPressed(Key_CursorLeft)  ) { *ax=-0.5; }
   if      (cpct_isKeyPressed(Key_CursorUp)    ) { *ay=-0.5; }
   else if (cpct_isKeyPressed(Key_CursorDown)  ) { *ay=0.5;  }

   // Q, A = Modify gravity
   if      (cpct_isKeyPressed(Key_Q)) {
      g_gravity += 0.01;
   } else  if (cpct_isKeyPressed(Key_A)) {
      g_gravity -= 0.01;
   } else {
      // Ensure Gravity message only appears when Q or A has been pressed
      return;
   }

   // Show a message with the new value of gravity
   strcpy(g_message.str, "Gravity:");
   concatNum(&g_message.str[8], g_gravity*100);
   g_message.time  = 25;
}

//
// Physics Example Main 
//
void main(void) {
   // Set up Logo entity
   TEntity logo = {
      gc_LogoFremos, CPCT_VMEM_START,  // Sprite and video memory location
      0, 0, 55, 20,                  // X, Y, Width and Height (bytes)
      { 0.5, 0.2, 0, 0, 1, 1 }       // Velocity values (vx, vy, acum_x,acum_y, max_x, max_y)
   };
   // Set up global message
   g_message.videopos = CPCT_VMEM_START;
   g_message.str[0]   = '\0';
   g_message.time     = 0;

   // Set up gravity
   g_gravity = 0.02;

   // We dont want the firmware to interfere with us
   cpct_disableFirmware();

   // Set up palette and border 
   cpct_fw2hw(gc_palette, 16);      // Convert firmware color values to hardware values
   cpct_setBorder(gc_palette[2]);   // Set the border
   cpct_setPalette(gc_palette, 16); // Set the palette

   // Set video mode to 0 (160x200, 16 colours)
   cpct_setVideoMode(0);

   // MAIN LOOP
   while(1) {
      f32 ax=0, ay=0;    // User acceleration values

      checkUserInput(&ax, &ay);
      updateEntities(&logo, ax, ay);
      drawMessage();
      cpct_drawSprite(logo.sprite, logo.videopos, logo.width, logo.height);
   }
}
