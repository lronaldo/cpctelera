//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
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
#include <stdio.h>
#include "tilemap.h"

/////////////////////////////////////////////////////////////////////////////////
// STRUCTURES AND GLOBALS
//

// Structure defining a viewport inside a Tilemap
typedef struct {
   u8 x, y; // Coordinates of the upper-left corner of the viewport relative to the tilemap
   u8 w, h; // Width and height in tiles of the viewport
} TViewport;

// Structure defining a tilemap located on the screen
typedef struct {
          u8 x, y;      // Location of the tilemap on the screen (byte coordinates)
   TViewport viewport;  // Viewport of the tilemap to show
} TScreenTilemap;


// Screen Buffers
//   0xC000 - Main Screen Buffer
//   0x8000 - BackBuffer (Requires moving program stack, that originally is at 0xBFFF)
//
u8* const g_scrbuffers[2] = { CPCT_VMEM_START, (u8*)0x8000 };

/////////////////////////////////////////////////////////////////////////////////
// Swaps between front-screen buffer and back-screen buffer. It manipulates 
// CRTC to change which buffer is shown at the screen, and then switches 
// the two buffers to make the reverse operation next time it gets called.
//
void swapBuffers(u8** scrbuffers) {
   u8* aux; // Auxiliary pointer for making the change
   
   // Change what is shown on the screen (present backbuffer (1) is changed to 
   // front-buffer, so it is shown at the screen)
   // cpct_setVideoMemoryPage requires the 6 Most Significant bits of the address,
   // so we have to shift them 10 times to the right (as addresses have 16 bits)
   //
   cpct_setVideoMemoryPage( (u16)(scrbuffers[1]) >> 10 );
   
   // Once backbuffer is being shown at the screen, we switch our two 
   // variables to start using (0) as backbuffer and (1) as front-buffer
   aux = scrbuffers[0];
   scrbuffers[0] = scrbuffers[1];
   scrbuffers[1] = aux;
}

/////////////////////////////////////////////////////////////////////////////////
// Wait for a given key to be pressed
//
void wait4Key(cpct_keyID key) {
   // First, if the key is already pressed, wait for 
   // it being released
   do
      cpct_scanKeyboard_f();
   while(cpct_isKeyPressed(key));

   // And now, wait for the key being pressed again
   do
      cpct_scanKeyboard_f();
   while(!cpct_isKeyPressed(key));
}

/////////////////////////////////////////////////////////////////////////////////
// Shows messages on how to use this program on the screen and waits
// until de user presses a key.
//
#define NUMMSGS  15
void showMessages() {
   u8 i;
   
   // Screen Messages
   static const u8* const messages[NUMMSGS] = { 
      "\017\002*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-",
      "\017\003             TILEMAPS DEMO\n\r", 
      "\017\002*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n\r",
      "\017\001Shows  a \017\003tilemap\017\001  through   a  viewport,",
      "letting you control the  \017\002location\017\001 of the",
      "\017\003tilemap\017\001 and the \017\002size\017\001 and \017\002position\017\001 of the",
      "viewport. All is done  using \017\003C\017\003P\017\003C\017\002telera\017\001's",
      "function  \017\002cpct_etm_drawTileBox2x4\017\001,  from",
      "its EasyTileMaps module.\n\r\n\r",
      "These are the \017\003control Keys\017\001:\n\r\n\r",
      "\017\002 Cursors \017\003-\017\001 Move tilemap location.\n\r",
      "\017\002  1, 2   \017\003-\017\001 Change viewport width.\n\r",
      "\017\002  3, 4   \017\003-\017\001 Change viewport height.\n\r",
      "\017\002 W,A,S,D \017\003-\017\001 Move viewport.\n\r\n\r",
      "       Press \017\002[\017\003Space\017\002]\017\001 to continue"
   };

   // Print all the messages 
   for (i=0; i < NUMMSGS; ++i)
      printf(messages [i]);

   // Wait for the user to press Space before continuing
   wait4Key(Key_Space);
}

/////////////////////////////////////////////////////////////////////////////////
// Read User Keyboard Input and do associated actions
//
void readKeyboardInput(TScreenTilemap *scr){
   
   // Read keyboard continuously until the user perfoms an action
   while(1) {
      // Scan Keyboard
      cpct_scanKeyboard_f(); 

      // Check all the user keys one by one and, if one of them is pressed
      // perform the action and return to the application
      //
      if (cpct_isKeyPressed(Key_CursorUp) && scr->y) {
         scr->y -= 4;   // Move Tilemap Up (4 by 4 pixels, as it can only be placed
         return;        // ... on pixel lines 0 and 4
      } else if (cpct_isKeyPressed(Key_CursorDown) && scr->y < (SCR_HEIGHT - 4*MAP_HEIGHT)) {
         scr->y += 4;   // Move Tilemap Down (same as moving Up, 4 by 4 pixels)
         return;
      } else if (cpct_isKeyPressed(Key_CursorLeft) && scr->x) {
         --scr->x;      // Move Tilemap Left 2 pixels (1 byte)
         return;
      } else if (cpct_isKeyPressed(Key_CursorRight) && scr->x < (SCR_WIDTH - 2*MAP_WIDTH)) {
         ++scr->x;      // Move Tilemap Right 2 pixels (1 byte)
         return;
      } else if (cpct_isKeyPressed(Key_2) && scr->viewport.x + scr->viewport.w < MAP_WIDTH) {
         ++scr->viewport.w;   // Enlarge viewport Horizontally
         return;
      } else if (cpct_isKeyPressed(Key_1) && scr->viewport.w > 1) {
         --scr->viewport.w;   // Reduce viewport Horizontally
         return;
      } else if (cpct_isKeyPressed(Key_4) && scr->viewport.y + scr->viewport.h < MAP_HEIGHT) {
         ++scr->viewport.h;   // Enlarge viewport Vertically
         return;
      } else if (cpct_isKeyPressed(Key_3) && scr->viewport.h > 1) {
         --scr->viewport.h;   // Reduce viewport Vertically
         return;
      } else if (cpct_isKeyPressed(Key_W) && scr->viewport.y) {
         --scr->viewport.y;   // Move viewport Up
         return;
      } else if (cpct_isKeyPressed(Key_S) && scr->viewport.y + scr->viewport.h < MAP_HEIGHT) {
         ++scr->viewport.y;   // Move viewport Down
         return;
      } else if (cpct_isKeyPressed(Key_A) && scr->viewport.x) {
         --scr->viewport.x;   // Move viewport Left
         return;
      } else if (cpct_isKeyPressed(Key_D) && scr->viewport.x + scr->viewport.w < MAP_WIDTH) {
         ++scr->viewport.x;   // Move viewport Right
         return;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////
// Draws the tilemap in a new location or with a change in its viewport.
//  The drawing is done in the backbuffer and then shown in the screen when it's
//  done, switching screen buffers just after the VSYNC
//
void drawScreenTilemap(TScreenTilemap *scr) {
   u8* ptmscr;    // Backbuffer pointer where the tilemap is to be drawn
   
   // Clear the backbuffer
   cpct_memset_f64(g_scrbuffers[1], 0x00, 0x4000);

   // Calculate the new location where the tilemap is to be drawn in
   // the backbuffer, using x, y coordinates of the tilemap
   ptmscr = cpct_getScreenPtr(g_scrbuffers[1], scr->x, scr->y);

   // Draw the viewport of the tilemap in the backbuffer (pointed by ptmscr)
   cpct_etm_drawTileBox2x4(scr->viewport.x, scr->viewport.y, 
                           scr->viewport.w, scr->viewport.h, 
                           MAP_WIDTH, ptmscr, g_tilemap);
   
   // Wait for VSYNC and change screen buffers just afterwards, 
   // to make the backbuffer show on the screen
   cpct_waitVSYNC();
   swapBuffers(g_scrbuffers);
}


/////////////////////////////////////////////////////////////////////////////////
// Main application's code
//
void application(void) {
   // Screen tilemap
   TScreenTilemap scr = { 0, 0, { 0, 0, MAP_WIDTH, MAP_HEIGHT} };

   // First show user messages
   showMessages();

   // Initialize the application
   cpct_disableFirmware();     // Firmware must be disabled for this application to work
   cpct_setBorder(0x00);       //    Set the border colour gray and.. 
   cpct_setPALColour(0, 0x14); // ...background black

   // VERY IMPORTANT: Before using EasyTileMap functions (etm), the internal
   // pointer to the tileset must be set. 
   cpct_etm_setTileset2x4(g_tileset);

   // Indefinitely draw the tilemap, listen to user input, 
   // do changes and draw it again
   while(1) {
      drawScreenTilemap(&scr);   // Redraws the tilemap
      readKeyboardInput(&scr);   // Waits for a user input and makes associated changes
   }
}

/////////////////////////////////////////////////////////////////////////////////
// MAIN PROGRAM:
//  Sets a new location for the stack and then calls the application code. 
//  Setting a new stack location must be done first, and the function doing it
//  must not use the stack (so, better not to use any local variable). Then,
//  it is preferable to just set it and call the application code.
//
void main(void) {
   // Move program's stack from 0xC000 to 0x8000. System return addresses are
   // already stored at 0xBFFA - 0xBFFF, but we don't care about them as our
   // program will never return to the system.
   cpct_setStackLocation((void*)0x8000);  

   // Start the application 
   application();   
}
