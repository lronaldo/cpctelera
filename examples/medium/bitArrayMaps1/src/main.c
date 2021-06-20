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
#include "constants.h"
#include "map.h"
#include "cursor.h"

/////////////////////////////////////////////////////////////////
// drawMessages
//    Draws some messages with the use instructions in the screen.
//
#define NUM_MSGS  6 
void drawMessages() {
   u8 i;
   
   // Define the strings for the messages as an array to 
   // be able to draw them in a single loop
   static u8* const strings[NUM_MSGS] = { 
       "[Cursors]" ,"Move"
      ,"[Space]"   ,"Draw/Remove"
      ,"[Escape]"  ,"Clear" 
   };

   // Define the properties of the messages that will be drawn:
   // (x,y) location, Foreground colour (fg) and Background Colour (bg)
   static const u8 msg_prop[NUM_MSGS][4] = {
    //   x,  y,     fg,      bg
    //----------------------------
      { 20,  0, C_RED , C_BLACK },  // Message 0
      { 40,  0, C_BLUE, C_BLACK },  // Message 1
      { 20, 16, C_RED , C_BLACK },  // Message 2
      { 40, 16, C_BLUE, C_BLACK },  // Message 3
      { 20, 32, C_RED , C_BLACK },  // Message 4
      { 40, 32, C_BLUE, C_BLACK }   // Message 5
   };

   // Draw all messages in a single loop, using propertis of each message
   for(i=0; i < NUM_MSGS; i++) {
      u8* pmem; 

      // Get Location where next message should be drawn using 
      // message properties 0 and 1 (x and y coordinates)
      pmem = cpct_getScreenPtr(CPCT_VMEM_START, msg_prop[i][0], msg_prop[i][1]);

      // Draw the i-th message with colours from its properties 2 and 3 (fg, bg)
      cpct_setDrawCharM1(msg_prop[i][2], msg_prop[i][3]);
      cpct_drawStringM1(strings[i], pmem);
   }
}
// Remove macro constant that is not required anymore
#undef NUM_MSGS

/////////////////////////////////////////////////////////////////
// initialize
//    Initialize this application when it starts. Disable firmware,
// set the video mode and colours, and initialize map and cursor.
//
void initialize() {
   u8 *pmem;

   // Disable firmware to prevent it from restoring video mode or
   // interfering with our drawString functions
   cpct_disableFirmware();

   // Set the video Mode to 1 (320x200, 4 colours)
   cpct_setVideoMode(1); 

   // Use default colours except for palette index 0 (background).
   // Default colours are (Blue, Yellow, Cyan, Red), let's use
   // (Black, Yellow, Cyan, Red). Change only colour 0 and border.
   cpct_setPALColour(0, 0x14);
   cpct_setBorder(0x14);
   
   // Initialize Base Pointer of the map in video memory. This is 
   // the place where the map will start to be drawn (0,0). This
   // location is (MAP_START_X, MAP_START_Y) with respect to CPCT_VMEM_START.
   pmem = cpct_getScreenPtr(CPCT_VMEM_START, MAP_START_X, MAP_START_Y);
   map_setBaseMem(pmem);

   // Set cursor at the top-left corner of the screen
   cursor_setLocation(0, 0);

   // Draw messages with instructions, the map and the cursor
   drawMessages();
   map_draw();
   cursor_draw();
}

/////////////////////////////////////////////////////////////////
// checkUserInputAndPerformActions
//    scans the keyboard and checks whether the user has pressed
// any action key or not. When the user presses an action key, 
// the corresponding action is performed.
//
void checkUserInputAndPerformActions() {
   // Scan the keyboard for user keypresses
   cpct_scanKeyboard();

   // Perform checks only when the user has
   // pressed at least one key
   if(cpct_isAnyKeyPressed()) {
      // Get current (x,y) coordinates of the cursor 
      u8 x = cursor_getX();
      u8 y = cursor_getY();

      // Check if the key pressed is one of the valid keys and
      // if sanity conditions are met

      // CURSOR_UP or CURSOR_DOWN (Only one at a time)
      //   Perform required action, if the cursor is not at the
      // edge of the map already.
      if (cpct_isKeyPressed(Key_CursorUp) && y > 0)
         cursor_move(DIR_UP);
      else if (cpct_isKeyPressed(Key_CursorDown) && y < MAP_HEIGHT-1)
         cursor_move(DIR_DOWN);

      // CURSOR_LEFT or CURSOR_RIGHT (Only one at a time)
      //   Perform required action, if the cursor is not at the
      // edge of the map already.
      if (cpct_isKeyPressed(Key_CursorLeft) && x > 0)
         cursor_move(DIR_LEFT);
      else if (cpct_isKeyPressed(Key_CursorRight) && x < MAP_WIDTH-1)
         cursor_move(DIR_RIGHT);

      // SPACE (change tile) or ESCAPE (clear map)
      //    Perform one of these actions if corresponding key is pressed
      if (cpct_isKeyPressed(Key_Space)) 
         map_changeTile(x, y);
      else if (cpct_isKeyPressed(Key_Esc))
         map_clear();

      // Always redraw the cursor after any of the above actions
      cursor_draw();
   }
}

/////////////////////////////////////////////////////////////////
//#############################################################//
/////////////////////////////////////////////////////////////////
// Main entry point of the application
//
void main (void) {
   // First of all, initialize the application
   initialize();

   // Main loop (repeat forever)
   while(1) {
      // Wait for VSYNC to limit actions to 60 per second
      cpct_waitVSYNC();
      
      // Check input and perform user actions
      checkUserInputAndPerformActions();
   }
}
