//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#ifndef _FRAME_H_
#define _FRAME_H_

#include <types.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//// FRAME 
////   Tiles and function used to draw the play area frame
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Function for drawing the frame using the tiles
void drawFrame(u8* pscr, u8 x);

// Tiles for defining the frame of the playing area
// by Dardalorth / Fremos / Carlio
extern const u8 G_frameUpLeftCorner[4*8];
extern const u8 G_frameUpRightCorner[4*8];
extern const u8 G_frameDownRightCorner[4*8];
extern const u8 G_frameDownLeftCorner[4*8];
extern const u8 G_frameLeft[4*8];
extern const u8 G_frameRight[4*8];
extern const u8 G_frameUp[4*8];
extern const u8 G_frameDown[4*8];
extern const u8 G_frameUpCenter[6*8];
extern const u8 G_frameDownCenter[6*8];
extern const u8 G_frameUpCenter2[2*8];
extern const u8 G_frameDownCenter2[2*8];

#endif