//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "globaldata.h"
#include "keyboard.h"
#include "draw.h"

/////////////////////////////////////////////////////////////////////////
// selectNextItem
//    Selects the next item and redraws the user interface
//
void selectNextItem() {
   // Next item is item + 1, except when we run out of items.
   // In that later case, we select again the first item (0)
   if (++g_selectedItem >= G_NITEMS)
      g_selectedItem = 0;

   // Draw changes
   drawUserInterfaceStatus();
}

/////////////////////////////////////////////////////////////////////////
// selectNextBlendMode
//    Selects the next blending mode and redraws the user interface
//
void selectNextBlendMode() {
   // Next blending mode is blendmode + 1, except when we run out of modes.
   // In that later case, we select again the first blending mode (0)
   if (++g_selectedBlendMode >= G_NBLENDMODES) 
      g_selectedBlendMode = 0;

   // Draw changes
   drawUserInterfaceStatus();
}

/////////////////////////////////////////////////////////////////////////
// performUserActions
//    Checks user input and performs selected actions
//
void performUserActions() {
   u8 i;

   // Checks status of every key in the g_keys array
   // Those keys with "Pressed" status trigger their associated action
   // Important: Pressed means "pressed just now". When the user maintains
   //            a key pressed, it moves to StillPressed status.
   for(i = 0; i < G_NKEYS; i++) {
      if (g_keys[i].status == KeySt_Pressed)
         g_keys[i].action();
   }
}

/////////////////////////////////////////////////////////////////////////
// Initialization routine
//    Disables firmware, initializes palette and video mode
//
void initialize (){ 
   // Disable firmware to prevent it from interfering
   cpct_disableFirmware();
   
   // 1. Set the palette colours using hardware colour values
   // 2. Set border colour to black
   // 3. Set video mode to 0 (160x200, 16 colours)
   cpct_setPalette  (g_palette, G_NCOLOURS);
   cpct_setBorder   (HW_BLACK);
   cpct_setVideoMode(0);

   // Draw the Background and the user interface
   drawUserInterfaceMessages();   
   drawBackground();

   // Set initial selections for item and blending mode
   g_selectedItem      = 0;
   g_selectedBlendMode = 0;
   drawUserInterfaceStatus();
}

/////////////////////////////////////////////////////////////////////////
// Main entry point of the application
//
void main(void) {
   initialize();  // Initialize the Amstrad CPC, 

   // Loop forever checking keyboard status and then
   // performing selected user actions
   while(1) {
      updateKeyboardStatus();
      performUserActions();
   }
}
