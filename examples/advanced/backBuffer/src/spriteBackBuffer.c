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
// SPRITE BACK BUFFER
//
// This file groups all functions and data used to perform drawing by using an sprite as
// back buffer. All drawing operations are performed from sprite to sprite (to the sprite 
// that is used as back buffer). When all drawings are finished, and after waiting to 
// VSYNC signal, the Sprite Back Buffer is then drawn as one big sprite to screen. This
// final unique operation is fast enough to avoid being caught by raster, then avoiding 
// flickering.
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#include <declarations.h>

////////////////////////////////////////////////////////////////////////////////////////////
// COPY TO SCREEN
//
//    Waits for VSYNC and then copies the Sprite Back Buffer to its final location at the
// screen (actually, it draws it)
//
void CopyToScreen() {
   // Calculate screen location where Sprite Back Buffer will be drawn
   u8*   pVideoMemLocation = GetScreenPtr(VIEW_X, VIEW_Y);

   // Wait for VSYNC and perform the actual drawing of the Sprite
   cpct_waitVSYNC();
   cpct_drawSprite(gBackBuffer, pVideoMemLocation, VIEW_W_BYTES, VIEW_H_BYTES);
}
