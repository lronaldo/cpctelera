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
// Mask Table Definition for Mode 1
cpctm_createTransparentMaskTable(gMaskTable, MASK_TABLE_LOCATION, M1, 0);

////////////////////////////////////////////////////////////////////////////////////////////
// CHECK USER INPUT
// 
//    Reads user input and changes selected draw function accordingly
//
void CheckUserInput() {
   cpct_scanKeyboard_f();
   
   if       (cpct_isKeyPressed(Key_1)) { SelectDrawFunction(1); DrawTextSelectionSign(1); }
   else if  (cpct_isKeyPressed(Key_2)) { SelectDrawFunction(2); DrawTextSelectionSign(2); }
   else if  (cpct_isKeyPressed(Key_3)) { SelectDrawFunction(3); DrawTextSelectionSign(3); }
}

////////////////////////////////////////////////////////////////////////////////////////////
// INITIALIZATION
// 
//    Initializes the CPC and all systems before starting the main loop
//
void Initialization() {
   // We need to disable firmware in order to set the palette and
   // to be able to use a second screen between 0x8000 and 0xBFFF
   cpct_disableFirmware(); 
   cpct_setPalette(g_palette, 5);   // Set the palette
   InitializeVideoMemoryBuffers();  // Initialize video buffers
   InitializeDrawing();             // Initialize Drawing Module
   SelectDrawFunction(1);           // Select the 1st Drawing function
   DrawTextSelectionSign(1);        // Mark 1st Drawing function as Selected
   DrawInfoText();                  // Draw User Info Text 
}

////////////////////////////////////////////////////////////////////////////////////////////
// MAIN PROGRAM
// 
void main(void) {
   // Change stack location before any call. We will be using
   // memory from 0x8000 to 0xBFFF as secondary buffer, so
   // the stack must not be there or it will get overwritten
   cpct_setStackLocation((u8*)NEW_STACK_LOCATION);
   
   // Initialize everything
   Initialization();
   
   // Main Loop
   while(1) {
      CheckUserInput();
      ScrollAndDrawSpace();
   }
}
