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
#include "sprites/sprites.h"
#include "sprites/frame.h"


//#define DEBUG

////////////////////////////////////////////////////////////////////////////////
// Initialization of the Amstrad CPC at the start of the game
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

////////////////////////////////////////////////////////////////////////////////
// Initializes the screen to start a new game
//
void initializeScreen(u16 hiscore) {
   u8* pscr;   // Pointer to the screen location where we want to draw
   u8  c;      // Colour pattern to draw
   u8  str[6]; // Score
   
   // Clear the screen
   cpct_clearScreen(0);

   // Draw limits
   c = cpct_px2byteM0(8,8);
   pscr = cpct_getScreenPtr(g_SCR_VMEM, 54,   0);
   cpct_drawSolidBox(g_SCR_VMEM, c,  4, 200);
   cpct_drawSolidBox(pscr      , c, 26, 200);
   pscr = cpct_getScreenPtr(g_SCR_VMEM,  4, 192);
   cpct_drawSolidBox(g_SCR_VMEM+4, c, 50, 8);
   c = cpct_px2byteM0(6,6);
   cpct_drawSolidBox(pscr        , c, 50, 8);

   pscr = cpct_getScreenPtr(g_SCR_VMEM, 60,  16);
   cpct_drawStringM0("HI", pscr, 3, 8);

   pscr = cpct_getScreenPtr(g_SCR_VMEM, 60,  24);
   sprintf(str, "%5u", hiscore);
   cpct_drawStringM0(str, pscr, 15, 8);

   drawFrame(g_SCR_VMEM, 0);
}

#ifdef DEBUG
   u8 g_stepByStep;
#endif

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

#ifdef DEBUG
   else if ( cpct_isKeyPressed(Key_D)  ) 
      g_stepByStep = 1;
   else if ( cpct_isKeyPressed(Key_N)  ) 
      g_stepByStep = 0;
#endif   

   else 
      performAction(user, es_static, user->side);
}

void waitNSyncs(u8 n) {
   if (n > 1) {
      while (--n) {
         cpct_waitVSYNC();
         __asm__("halt");
         __asm__("halt");
      }
   }
   cpct_waitVSYNC();
}

void wait4Key(cpct_keyID key) {
   do
      cpct_scanKeyboard_f();
   while( ! cpct_isKeyPressed(key) );
   do
      cpct_scanKeyboard_f();
   while( cpct_isKeyPressed(key) );
}


////////////////////////////////////////////////////////////////////////////////
// Plays a complete game, until the death of the character
//
u16 game(u16 hiscore) {
   u8 alive = 1;
   TCharacter* c;

#ifdef DEBUG
   g_stepByStep = 0;
#endif 

   // Initialize game
   initializeScreen(hiscore);
   initializeEntities();
      
   c = getCharacter();

   // Main Game Loop
   while(alive) {
      updateUser(c);
      scrollWorld();
      alive = updateCharacter(c);
      cpct_waitVSYNC();
      drawAll();
#ifdef DEBUG
      if (g_stepByStep)
         wait4Key(Key_X);
#endif
   }

   return getScore();
}

////////////////////////////////////////////////////////////////////////////////
// Plays a complete game, until the death of the character
//
void showGameEnd(u16 score) {
   u8* pscr;   // Pointer to the screen place where to show messages
   u8  str[6];

   pscr = cpct_getScreenPtr(g_SCR_VMEM,  8, 24);
   cpct_drawStringM0("GAME  OVER", pscr, 6, 0);

   pscr = cpct_getScreenPtr(g_SCR_VMEM, 16, 48);
   cpct_drawStringM0(  "SCORE", pscr, 9, 0);

   pscr = cpct_getScreenPtr(g_SCR_VMEM, 16, 56);
   sprintf(str, "%5u", score);
   cpct_drawStringM0(str, pscr, 14, 0);

   pscr = cpct_getScreenPtr(g_SCR_VMEM, 6, 112);
   cpct_drawStringM0("PRESS SPACE", pscr, 11, 0);

   do
      cpct_scanKeyboard_f();
   while ( !cpct_isKeyPressed(Key_Space) );
}

//////////////////////////////////////////////////////////////////////
// MAIN EXAMPLE CODE
//    Keys:
//
void main(void) {
   u16 score, hi = 0;

   initializeCPC();

   // End of the game
   while(1) {
      score = game(hi);
      if (score > hi)
         hi = score;

      showGameEnd(score);
   }
}
