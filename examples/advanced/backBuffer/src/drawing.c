//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2017 Bouche Arnaud
//  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#include <declarations.h>

/////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
u8 gNbTileset;       // Nb tileset of star background
u8 gPosScroll;       // Pos scrolling of star background
u8 gDrawFunction;    // Selected draw function
TDrawFunc gDrawFunc; // Pointer to the current drawing function


/////////////////////////////////////////////////////////////////////////////////
// INITIALIZE DRAWING
//
//    Initializes global variables and general status for drawing functions
//
void InitializeDrawing() {
   gNbTileset = sizeof(g_tileset)/sizeof(u8*);
   gPosScroll = 0;
   SelectDrawFunction(1);
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW DIRECTLY TO SCREEN
//
//    Draws the scene directly to video memory using simple drawSprite functions
// with no back buffer.
//
void DrawDirectlyToScreen() {
   u8* screenPtr;
   u8 i;

   cpct_waitVSYNC();
   
   // Draw Tiled Background. All tiles are drawn, starting by gPosScroll
   // which changes each frame, therefore simulating a scrolling effect
   //
   for(i = 0; i < VIEW_W_BYTES; i++) {
      // Get next tile to draw and calculate its screen location
      u8 tile = (gPosScroll + i) % gNbTileset; 
      screenPtr = GetScreenPtr(VIEW_X + i, VIEW_Y);

      // Draw the tile in the screen
      cpct_drawSprite(g_tileset[tile], screenPtr, G_BACK_00_W, G_BACK_00_H);
   }

   // Draw title at fixed coordinates. It is a masked sprite that 
   // will be upfront the tiles, in the middle
   //
   screenPtr = GetScreenPtr(VIEW_X + POS_TITLE_X, 0);
   cpct_drawSpriteMaskedAlignedTable(g_title, screenPtr, G_TITLE_W, G_TITLE_H, gMaskTable);

   // Draw the ship
   // It is also a masked sprite that will be upfront the tiles, in the middle
   //
   screenPtr = GetScreenPtr(VIEW_X + POS_SHIP_X, VIEW_Y + POS_SHIP_Y);
   cpct_drawSpriteMasked(g_ship, screenPtr, G_SHIP_W, G_SHIP_H);
   
   // Draw the fire produced by ship's engine. Fire is an animation
   // that has 2 different sprites. We open a new scope in order to 
   // define local variables comfortably.
   //
   {
      // Select sprite and calculate its video memory location
      const u8* fireSp = (gPosScroll % 2) ? g_fire_0 : g_fire_1;
      u8  x = VIEW_X + POS_SHIP_X - G_FIRE_0_W;
      u8  y = VIEW_Y + POS_SHIP_Y + 2;
      screenPtr = GetScreenPtr(x, y);
      
      // Then draw the fire at the end of the ship
      cpct_drawSpriteMaskedAlignedTable(fireSp, screenPtr, G_FIRE_0_W, G_FIRE_0_H, gMaskTable);
   }
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW USING HARDWARE BACK BUFFER
//
//    Draws the scene directly to the hardware back buffer that is not being
// displayed. This way, nothing happens on the screen while drawing is being
// performed. After drawing it flips buffers and then the whole image is displayed.
//
void DrawUsingHardwareBackBuffer() {
   u8* backScreenPtr;
   u8 i;

   // Draw Tiled Background. All tiles are drawn, starting by gPosScroll
   // which changes each frame, therefore simulating a scrolling effect
   //
   for(i = 0; i < VIEW_W_BYTES; i++) {
      // Get next tile to draw and calculate its screen location
      u8 tile = (gPosScroll + i)%gNbTileset; 
      backScreenPtr = GetBackBufferPtr(VIEW_X + i, VIEW_Y);
      
      // Draw the tile in the hardware back buffer
      cpct_drawSprite(g_tileset[tile], backScreenPtr, G_BACK_00_W, G_BACK_00_H);     
   }

   // Draw title at fixed coordinates in the hardware back buffer. 
   // It is a masked sprite that will be upfront the tiles, in the middle
   //
   backScreenPtr = GetBackBufferPtr(VIEW_X + POS_TITLE_X, 0);
   cpct_drawSpriteMaskedAlignedTable(g_title, backScreenPtr, G_TITLE_W, G_TITLE_H, gMaskTable);
   
   // Draw the ship in the hardware back buffer. 
   // It is also a masked sprite that will be upfront the tiles, in the middle
   //
   backScreenPtr = GetBackBufferPtr(VIEW_X + POS_SHIP_X, VIEW_Y + POS_SHIP_Y);
   cpct_drawSpriteMasked(g_ship, backScreenPtr, G_SHIP_W, G_SHIP_H);

   // Draw the fire produced by ship's engine to the hardware back buffer. 
   // Fire is an animation that has 2 different sprites. We open a 
   // new scope in order to define local variables comfortably.
   //
   {
      const u8* fireSp = (gPosScroll % 2) == 0 ? g_fire_0 : g_fire_1;
      u8  x = VIEW_X + POS_SHIP_X - G_FIRE_0_W;
      u8  y = VIEW_Y + POS_SHIP_Y + 2;
      backScreenPtr = GetBackBufferPtr(x, y);
      
      cpct_drawSpriteMaskedAlignedTable(fireSp, backScreenPtr, G_FIRE_0_W, G_FIRE_0_H, gMaskTable);
   }
     
   // Flip buffers to display the present back buffer
   // and stop displaying the current video memory
   FlipBuffers();
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW USING SPRITE BACK BUFFER
//
//    Draws the scene to a sprite that acts as software back buffer. As all 
// drawing operations are performed internally, outside video memory, nothing 
// happens on the screen while drawing to sprite is being performed. After 
// drawing to sprite, it performs a single draw to screen operation, that
// draws the complete sprite that acts as back buffer. This drawing operation
// is fast enough not to be caught by the raster, and does not produce
// any flickering.
//
void DrawUsingSpriteBackBuffer() {
   u8* backBufferPtr;
   u8 i;

   // Draw Tiled Background. All tiles are drawn, starting by gPosScroll
   // which changes each frame, therefore simulating a scrolling effect
   //
   for(i = 0; i < VIEW_W_BYTES; i++) {
      // Get next tile to draw and calculate its screen location
      u8 tile = (gPosScroll + i)%gNbTileset;
      backBufferPtr = GetSpriteBackBufferPtr(i, 0);
      
      // Draw the tile to the sprite that acts as back buffer
      cpct_drawToSpriteBuffer(VIEW_W_BYTES, backBufferPtr, G_BACK_00_W, G_BACK_00_H, g_tileset[tile]);
   }

   // Draw title at fixed coordinates, in the sprite that acts as back buffer. 
   // It is a masked sprite that will be upfront the tiles, in the middle
   //
   backBufferPtr = GetSpriteBackBufferPtr(POS_TITLE_X, 0);
   cpct_drawToSpriteBufferMaskedAlignedTable(VIEW_W_BYTES, backBufferPtr, G_TITLE_W, G_TITLE_H, g_title, gMaskTable);
   
   // Draw the ship in the sprite that acts as software back buffer
   // It is also a masked sprite that will be upfront the tiles, in the middle
   //
   backBufferPtr = GetSpriteBackBufferPtr(POS_SHIP_X, POS_SHIP_Y);
   cpct_drawToSpriteBufferMasked(VIEW_W_BYTES, backBufferPtr, G_SHIP_W, G_SHIP_H, g_ship);

   // Draw the fire produced by ship's engine to the sprite that acts
   // as back buffer. Fire is an animation that has 2 different sprites. 
   // We open a new scope in order to define local variables comfortably.
   //
   {
      const u8* fireSp = (gPosScroll % 2) == 0 ? g_fire_0 : g_fire_1;
      u8  x = POS_SHIP_X - G_FIRE_0_W;
      u8  y = POS_SHIP_Y + 2;
      backBufferPtr = GetSpriteBackBufferPtr(x, y);
      
      cpct_drawToSpriteBufferMaskedAlignedTable(VIEW_W_BYTES, backBufferPtr, G_FIRE_0_W, G_FIRE_0_H, fireSp, gMaskTable);
   }

   // Copy Sprite Back Buffer to screen memory
   DrawSpriteBackBufferToScreen();
}

/////////////////////////////////////////////////////////////////////////////////
// SELECT DRAW FUNCTION
//
//    Selects the function that will be used for drawing purposes
//
void SelectDrawFunction(u8 drawFuncNb) {
   // Select draw function depending on its number
   switch(drawFuncNb) {
      case 1:  gDrawFunc = DrawDirectlyToScreen;         break;
      case 2:  gDrawFunc = DrawUsingHardwareBackBuffer;  break;
      default: gDrawFunc = DrawUsingSpriteBackBuffer;
   }
}

/////////////////////////////////////////////////////////////////////////////////
// SCROLL AND DRAW SPACE
//
//    Advances the scroll a bit to the right and then draws the space animation 
// using the selected Drawing Function. It does so by simply calling the current 
// drawing function.
//
void ScrollAndDrawSpace() { 
   // Scroll background, draw title, draw ship
   gPosScroll++;
   gDrawFunc(); 
}