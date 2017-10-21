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

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// SCENE DRAWING FUNCTIONS
//
// This module groups functions and data structures used to draw the main demonstrative
// scene in this demo. It can draw the same scene using 3 different functions:
//    - DrawDirectlyToScreen: draws everything directly to the displayed video memory
//    - DrawUsingHardwareBackBuffer: draws to a hardware back buffer, switching buffers
//          when drawing is finished.
//    - DrawUsingSpriteBackBuffer: draws to a Sprite used as back buffer first. Then, 
//          it draws the sprite back buffer directly to displayed video memory in 1 single step.
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRAWING_H_
#define _DRAWING_H_

////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DEFINED TYPES
//

// Draw functions pointer (Points to any of the drawing functions)
typedef void (*TDrawFunc)(void);


////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION DECLARATIONS
//
void  InitializeDrawing();
void  SelectDrawFunction(u8 drawFuncNb);
void  ScrollAndDrawSpace();

#endif