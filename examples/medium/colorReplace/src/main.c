//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2021 Bouche Arnaud (@Arnaud6128)
//  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#include "declarations.h"

/////////////////////////////////////////////////////////////////////////////////
// Mask Table Definition for Mode 0
//
cpctm_createTransparentMaskTable(gMaskTable, MASK_TABLE_LOC, M0, 0);

///////////////////////////////////////////////////////
/// INITIALIZATION
/// 
//    Initializes the CPC and all systems before starting the main loop
//
void Initialization()
{
    // We need to disable firmware in order to set the palette and
    // to be able to use a second screen between 0x8000 and 0xBFFF
    cpct_disableFirmware();
    cpct_setVideoMode(0);            // Set mode 0
    cpct_setPalette(g_palette, 16);  // Set the palette
    cpct_setBorder(HW_SKY_BLUE);     // Set the border color with Hardware color
    
    InitializeVideoMemoryBuffers();  // Initialize video buffers    
    InitializeDrawing();             // Initialize drawing elements
}

///////////////////////////////////////////////////////
/// MAIN PROGRAM
/// 
void main(void) 
{
    // Change stack location before any call. We will be using
    // memory from 0x8000 to 0xBFFF as secondary buffer, so
    // the stack must not be there or it will get overwritten
    cpct_setStackLocation((u8*)NEW_STACK_LOC);
    
    // Initialize everything
    Initialization();
    
    // Main Loop
    while (TRUE)
    {
        UpdateBalloons();
        DrawSceneBalloons();
        DrawStars();
        
        // Flip buffers to display the present back buffer
        // and stop displaying the current video memory
        FlipBuffers();        
    }
}
