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

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// MANAGE VIDEO MEMORY
//
// This module groups functions and variables that control video memory. These functions
// take care of which memory is being used as screen video memory and which as hardware 
// back buffer. They also switch both buffers on demand.
// This module also controls the function tu perform drawing from a sprite back buffer
// to video memory. When all drawings are finished, and after waiting to VSYNC signal, 
// the Sprite Back Buffer is then drawn as one big sprite to screen. This final unique 
// operation is fast enough to avoid being caught by raster, then avoiding flickering.
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MANAGEVIDEOMEM_H_
#define _MANAGEVIDEOMEM_H_

#include <cpctelera.h>

////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION DECLARATIONS
//
void  InitializeVideoMemoryBuffers  ();
u8*   GetScreenPtr                  (u8 xPos, u8 yPos);
u8*   GetBackBufferPtr              (u8 xPos, u8 yPos);
void  FlipBuffers                   ();

#endif