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

////////////////////////////////////////////////////////////////////////////////////////
// DRAW INFO TEXT TO BUFFER
//
//    Draws all information texts into a given hardware screen buffer. This information
// is a guide for the user to know how this demo works
//
// INPUT: 
//    bufferPtr = pointer to the start of the harware screen buffer
//
void DrawInfoTextToBuffer(u8* bufferPtr) {
   // Define the 10 messages to draw, along with their
   // on-sreen locations and colours
   #define NB_MESSAGES  10
   static const TTextData messages[NB_MESSAGES] = {
      //--------------------------------------------------------------------
      // |  X |      Y        | PEN | PAPER |           TEXT               |
      //--------------------------------------------------------------------
         {  0 , POS_TEXT -  5 ,  3  ,   0   , "Press"                      }
      ,  {  4 , POS_TEXT + 15 ,  1  ,   0   , "1"                          }
      ,  {  8 , POS_TEXT + 15 ,  2  ,   0   , ": No double buffer"         }
      ,  {  8 , POS_TEXT + 25 ,  1  ,   0   , "Directly draw in video mem" }
      ,  {  4 , POS_TEXT + 40 ,  1  ,   0   , "2"                          }
      ,  {  8 , POS_TEXT + 40 ,  2  ,   0   , ": Hardware double buffer"   }      
      ,  {  8 , POS_TEXT + 50 ,  1  ,   0   , "Draw alternatively in two video mem  (2*16384 bytes) and flip between them" }
      ,  {  4 , POS_TEXT + 75 ,  1  ,   0   , "3"                          }
      ,  {  8 , POS_TEXT + 75 ,  2  ,   0   , ": Software double buffer"   }
      ,  {  8 , POS_TEXT + 85 ,  1  ,   0   , "Draw in buffer (50*60 bytes) of size view and copy whole buffer to video mem (16384 bytes)" }
   };
   u8 i;

   // Loop through all the messages and draw them one by one
   for (i = 0; i < NB_MESSAGES; ++i) {
      // Get a pointer to the Text Data
      TTextData const* t = messages + i;
      
      // Calculate on-screen location where to draw the message 
      // and draw it with the selected colours
      u8* pvmem = cpct_getScreenPtr(bufferPtr, t->x, t->y);
      cpct_setDrawCharM1(t->pen, t->paper);
      cpct_drawStringM1(t->text, pvmem);
   }
}

////////////////////////////////////////////////////////////////////////////////////////
// DRAW INFO TEXT 
//
//    Draws user information texts both on Screen Video Memory and Hardware back buffer.
// 
void DrawInfoText() {
   DrawInfoTextToBuffer((u8*)CPCT_VMEM_START);
   DrawInfoTextToBuffer((u8*)SCREEN_BUFF);
}


////////////////////////////////////////////////////////////////////////////////////////
// DRAW SELECTION TO BUFFER
//
//    Removes previous signs and draws a new one near the user selection in the given
// hardware screen buffer.
// 
void DrawSelectionToBuffer(u8* bufferPtr, u8 pos) {
   u8* pvmem;  // Pointer to video memory

   // Clear previous selection by drawing a background-colour box
   // over the whole region where selections are drawn
   pvmem = cpct_getScreenPtr(bufferPtr, 0, POS_TEXT + 15);
   cpct_drawSolidBox(pvmem, 0, 2, 80);

   // Then draw the selection sign near the user selected item
   pvmem = cpct_getScreenPtr(bufferPtr, 0, pos);
   cpct_setDrawCharM1(3, 0);
   cpct_drawStringM1(">", pvmem);
}

////////////////////////////////////////////////////////////////////////////////////////
// DRAW TEXT SELECTION SIGN
//
//    Draws a sign near the currently selected option in both screen buffers (screen 
// video memory and hardware back buffer)
// 
void DrawTextSelectionSign(u8 sel) {
   // On-screen Y locations of the 3 selections the
   // user can pick up
   static const u8 locations[3] = { 
         POS_TEXT + 15
      ,  POS_TEXT + 40
      ,  POS_TEXT + 75
   };
   u8 pos = locations[sel-1];   // Position of the User selection
   
   // Draw text in both video buffer 
   DrawSelectionToBuffer((u8*)CPCT_VMEM_START, pos);
   DrawSelectionToBuffer((u8*)SCREEN_BUFF, pos);
}
