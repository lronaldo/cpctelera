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
#include "scifi_bg.h"
#include "items.h"

/////////////////////////////////////////////////////////////////////////
// drawBackground
//    Draws the background from pixel line 72 onwards
//
void drawBackground() {
   // Get a pointer to the (x,y) location in the screen where 
   // the background has to be drawn (its upper-left corner)
   u8* p = cpct_getScreenPtr(CPCT_VMEM_START, BG_X, BG_Y);

   // The sprite of the background is split into 2 spites of half
   // its wide, to bypass the limit of 63-bytes that drawSprite can draw.
   // So, we have to draw them both. The first on (x,y) and the second
   // on (x + BG_WIDTH/2, y)
   cpct_drawSprite(g_scifi_bg_0, p             , BG_WIDTH/2, BG_HEIGHT);
   cpct_drawSprite(g_scifi_bg_1, p + BG_WIDTH/2, BG_WIDTH/2, BG_HEIGHT);
}

/////////////////////////////////////////////////////////////////////////
// drawSpriteMixed
//    draws a (sprite) of size (width,height) at a given location (x,y) using
// a given blending (mode)
//
void drawSpriteMixed(  CPCT_BlendMode mode, u8* sprite
                     , u8 x, u8 y, u8 width, u8 height) {
   // Get a pointer to the (x,y) location in the screen where
   // the sprite will be drawn (its upper-left corner)
   u8* p = cpct_getScreenPtr(CPCT_VMEM_START, x, y);

   // Set the blend mode to use before drawing the sprite using blending
   cpct_setBlendMode(mode);

   // Draw the sprite to screen with blending
   cpct_drawSpriteBlended(p, height, width, sprite);
}

/////////////////////////////////////////////////////////////////////////
// drawCurrentSpriteAtRandom
//    draws the current selected sprite using the current selected blending
// mode at a random (x,y) location inside the map
//
void drawCurrentSpriteAtRandom() {
   u8 x, y;

   // Select 2 random coordinates to put the sprite inside the background
   // Take into account the width (4) and height (8) of sprites in bytes
   // not to put them too close to the boundaries
   x = BG_X + ( cpct_rand() % (BG_WIDTH  - 4) );
   y = BG_Y + ( cpct_rand() % (BG_HEIGHT - 8) );

   // Draw the selected item with selected blending mode
   drawSpriteMixed(  g_blendModes[g_selectedBlendMode].blendmode
                   , g_items[g_selectedItem].sprite
                   , x, y, 4, 8);
}

/////////////////////////////////////////////////////////////////////////
// drawUserInterfaceStatus
//    draws the current user interface status values: the selected item
// and the selected blending mode
//
void drawUserInterfaceStatus() {
   // Get a pointer to the (x,y) location in the screen where
   // the name and sprite of the item will be drawn
   u8 *p = cpct_getScreenPtr(CPCT_VMEM_START, 4, 60);
   
   // Select foreground/background colours for drawStringM0 functions
   cpct_setDrawCharM0(8, 0);

   // Draw the name of the item and its corresponding sprite
   // The sprite has to be drawn 7 characters to the right (7*4 = 28 bytes)
   cpct_drawStringM0(g_items[g_selectedItem].name   , p       );
   cpct_drawSprite  (g_items[g_selectedItem].sprite , p + 28 , 4, 8);
   
   // Do the same as before to draw the name of the current blending mode
   p = cpct_getScreenPtr(CPCT_VMEM_START, 52, 60);
   cpct_drawStringM0(g_blendModes[g_selectedBlendMode].name, p);
}

/////////////////////////////////////////////////////////////////////////
// drawUserInterfaceMessages
//    draws the messages that conform the user interface 
//
void drawUserInterfaceMessages() {
   u8 *p;   // Pointer to screen video memory

   // Draw first two strings at the top-left corner of the screen
   // (exactly at CPCT_VMEM_START) and 8 characters to the right (8*4 = 32 bytes)
   cpct_setDrawCharM0(3, 0);
   cpct_drawStringM0("[Space]"   , CPCT_VMEM_START    );
   cpct_setDrawCharM0(9, 0);
   cpct_drawStringM0("Draw Item" , CPCT_VMEM_START+32 );
   
   // Get a pointer to the first pixel in the 15th line of the screen
   // And draw there next 2 strings, being the second 8 characters to the right also
   p = cpct_getScreenPtr(CPCT_VMEM_START, 0, 15);
   cpct_setDrawCharM0(3, 0);
   cpct_drawStringM0("[1] [2]"   , p    );
   cpct_setDrawCharM0(9, 0);
   cpct_drawStringM0("Select"    , p+32 );

   // Repeat same operation as before, but to draw at the start of the 30th line
   p = cpct_getScreenPtr(CPCT_VMEM_START, 0, 30);
   cpct_setDrawCharM0(3, 0);
   cpct_drawStringM0("[Esc]"     , p    );
   cpct_setDrawCharM0(9, 0);
   cpct_drawStringM0("Clear"     , p+32 );

   // And to same operation again, but to put messages for 
   // selected Item and Blend mode on the 50th line of the screen
   p = cpct_getScreenPtr(CPCT_VMEM_START, 0, 50);
   cpct_setDrawCharM0(1, 6);
   cpct_drawStringM0("   Item     Blend   ", p);
}
