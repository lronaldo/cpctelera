//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 Arnaud Bouche
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

/////////////////////////////////////////////////////////////////////////////////
// INCLUDES
#include <cpctelera.h>

/////////////////////////////////////////////////////////////////////////////////
// SPRITES
#include "sprites/ufo.h"
#include "sprites/building_1.h"
#include "sprites/building_2.h"
#include "sprites/building_3.h"

/////////////////////////////////////////////////////////////////////////////////
// USEFUL MACROS AND CONSTANTS
//
#define BOOL         u8
#define TRUE         1
#define FALSE        0

#define SCREEN_CY    200
#define SCREEN_CX    80
#define VMEM_SIZE    0x4000

#define UFO_Y        120
#define UFO_INIT_X   0
#define GRADIENT_CY  10

/////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES

// This array will contain a copy of the background in the video memory region that
// we will be overwritting with the UFO sprite. This copy will let us erase the 
// UFO sprite restoring the previous background.
u8 gScreenCapture[ G_UFO_0_H * G_UFO_0_W ];  

/////////////////////////////////////////////////////////////////////////////////
// INITIALIZATION
// 
void Initialization() {
   cpct_disableFirmware();            // Disable firmware to take full control of the CPC
   cpct_setVideoMode(0);              // Set mode 0
   cpct_setPalette (g_palette, 16);   // Set the palette
   cpct_setBorder(HW_BLACK);          // Set the border color with Hardware color
   
   // Fill screen with color index 4 (Red)
   cpct_memset(CPCT_VMEM_START, cpct_px2byteM0(4, 4), VMEM_SIZE);
}

/////////////////////////////////////////////////////////////////////////////////
// GET UFO CURRENT ANIMATION SPRITE
//
u8* GetUfoSprite() {
   // Private UFO Animation-Status Data
   static u8 anim = 0;     // Currently selected animation sprite
   static u8* const ufoSprite[] = { 
      g_ufo_0, g_ufo_1, g_ufo_2, g_ufo_3  // Array of all four animation sprites
   };
    
   // Just get next animation sprite and return it
   return ufoSprite[anim++ % 4];
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW UFO
//
void DrawUFO() {
    // Private data to control UFO location and status
   static BOOL moveRight;  // Keep move direction
   static u8 posX;         // Keep position X 
   
   // Get a pointer to the start of the UFO sprite in video memory
   // previous to moving. Background will need to be restored at this 
   // precise location to erase UFO before drawing it in its next location
   u8* pvmem_ufoBg = cpct_getScreenPtr(CPCT_VMEM_START, posX, UFO_Y);
   
   // Temporal pointer to videomemory for drawing sprites (will be used later)
   u8* pvmem;

   // UFO Moves towards right or left
   // When border is reached, movement direction changes
   if (moveRight) {
      // If right border reached go to left border
      if (posX == SCREEN_CX - G_UFO_0_W)
         moveRight = FALSE;   // Change direction
      else 
         posX++;              // Move to right
   } else {
      // If left border reached go to right border
      if (posX == 0)  moveRight = TRUE;   // Change direction
      else            posX--;             // Move to left
   }
   
   // Get a pointer to the Screen Video Memory location where 
   // UFO will be drawn next (in its new location after movement)
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, posX, UFO_Y);
   
   // Wait for VSync and draw background and UFO
   // This drawing operation is fast enough not to be caught by the raster, and does not produce
   // any flickering. In other case you have to use double-buffer to prevent this.
   cpct_waitVSYNC();
   
   //--- UFO REDRAWING
    
   // Erase UFO at its previous location by drawing background over it
   cpct_drawSprite(gScreenCapture, pvmem_ufoBg, G_UFO_0_W, G_UFO_0_H);
   
   // Before drawing UFO at its new location, copy the background there
   // to gScreenCapture buffer. This will let us restore it next time
   // the UFO moves.
   cpct_getScreenToSprite(pvmem, gScreenCapture, G_UFO_0_W, G_UFO_0_H);
   
   // Draw UFO at its new location
   cpct_drawSpriteMasked(GetUfoSprite(), pvmem, G_UFO_0_W, G_UFO_0_H);
}

/////////////////////////////////////////////////////////////////////////////////
// FILL LINE OF COLOR
//
void FillLine(u8 pixColor, u8 lineY) {
   // Get a pointer to the start of line Y of screen video memory
   // and fill it with given colour 
   u8* pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 0, lineY);
   cpct_memset(pvmem, pixColor, SCREEN_CX);
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW SKY PART FILLED WITH GRADIENT
//
u8 DrawSkyGradient(u8 cy, u8 posY, u8 colorFront, u8 colorBack) {
   u8 i, j;
   
   // Get Mode 0 screen pixel colour representations for front and back colours
   u8 pixFront = cpct_px2byteM0(colorFront, colorFront);
   u8 pixBack  = cpct_px2byteM0(colorBack, colorBack);

   // Draw gradient zone
   for (j = 0; j < cy; j++) {
      // If end of screen reached stop drawing
      if (posY == SCREEN_CY - 2)
         break;

      // Draw lines of color
      for (i = 0; i < cy - j; i++) {
         FillLine(pixFront, posY++);      
      }  
      FillLine(pixBack, posY++); 
   }
   
   // Return ending line colorized
   return posY;
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW SKY WITH GRADIENT ZONES FOR BACKGROUND
//
void DrawSky() {
   // Define color of gradient sky parts
   static const u8 colors[] = { 2, 15, 2, 7, 10, 13, 8, 4 };
   
   // Current line filled with color
   u8 startLine = 0;
      
   // Screen is divided into gradient zone
   u8 i;
   for (i = 1; i < sizeof(colors); i++)
      startLine = DrawSkyGradient(GRADIENT_CY - i, startLine, colors[i], colors[i - 1]);
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW CITY WITH ALL BUILDING FOR BACKGROUND
//
void DrawCity() {
   u8* pvmem;

   // As all locations of buildings are constant, we use cpctm_screenPtr that 
   // will calculate screen video memory pointers during compile time (as it
   // is a macro). Resulting assembly code will be shorter and faster, as 
   // it will only contain exact video memory values for pointers, instead of
   // code for calculating them.
   pvmem = cpctm_screenPtr(CPCT_VMEM_START, 10, SCREEN_CY - G_BUILDING_1_H);
   cpct_drawSprite(g_building_1, pvmem, G_BUILDING_1_W, G_BUILDING_1_H);
   
   pvmem = cpctm_screenPtr(CPCT_VMEM_START, 30, SCREEN_CY - G_BUILDING_2_H);
   cpct_drawSprite(g_building_2, pvmem, G_BUILDING_2_W, G_BUILDING_2_H);
   
   pvmem = cpctm_screenPtr(CPCT_VMEM_START, 40, SCREEN_CY - G_BUILDING_1_H);
   cpct_drawSprite(g_building_1, pvmem, G_BUILDING_1_W, G_BUILDING_1_H);
   
   pvmem = cpctm_screenPtr(CPCT_VMEM_START, 67, SCREEN_CY - G_BUILDING_2_H);
   cpct_drawSprite(g_building_2, pvmem, G_BUILDING_2_W, G_BUILDING_2_H);
   
   pvmem = cpctm_screenPtr(CPCT_VMEM_START, 60, SCREEN_CY - G_BUILDING_3_H);
   cpct_drawSprite(g_building_3, pvmem, G_BUILDING_3_W, G_BUILDING_3_H);
}

/////////////////////////////////////////////////////////////////////////////////
// INITIALIZE FIRST BACKGROUND CAPTURE    
// 
void InitCapture() {
   // Get Screen Video Memory pointer of default UFO location 
   // and make a copy of the background pixel data there to gScreenCapture buffer
   u8* pvmem = cpctm_screenPtr(CPCT_VMEM_START, UFO_INIT_X, UFO_Y);
   cpct_getScreenToSprite(pvmem, gScreenCapture, G_UFO_0_W, G_UFO_0_H);
}

/////////////////////////////////////////////////////////////////////////////////
// DRAW BACKGROUND WITH GRADIENT SKY AND BUILDING     
// 
void DrawBackground() {
   DrawSky();
   DrawCity();
   
   InitCapture();
}

///////////////////////////////////////////////////////
/// MAIN PROGRAM
/// 
void main(void)  {
   Initialization();   // Initialize everything
   DrawBackground();   // Draw background with sky and buildings
   
   // Main Loop: Permanently move and draw the UFO
   while(1) {
      DrawUFO();
   }
}
