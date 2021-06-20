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
// TEXT DRAWING FUNCTIONS
//
// This module groups functions and data structures used to draw textual information 
// on the screen.
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TEXTDRAWING_H_
#define _TEXTDRAWING_H_

#include <cpctelera.h>

////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA STRUCTURE DECLARATIONS
//

//
// TTextData
//
//    Data elements used to create a table of text, along with their on-screen location 
// coordinates (x,y) and drawing colour (pen, paper).
//
typedef struct {
   u8          x, y, pen, paper;
   char const *text;
} TTextData;

////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION DECLARATIONS
//
void DrawInfoText();
void DrawTextSelectionSign(u8 sel);

#endif