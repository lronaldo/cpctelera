//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
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

#include <cpctelera.h>

void main(void) {
   u8* pvmem;  // Pointer to video memory

   // Make pvmem point to the byte in video memory where we want
   // to print our string (coordinates (20, 96) in bytes, (80, 96) in pixels,
   // as each byte is 4 pixels wide in mode 1)
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 20, 96);
   // Set Mode 1 Character&String drawing functions to use colours 
   // 1 (yellow) for foreground and 0 (blue) for background
   cpct_setDrawCharM1(1, 0);
   // Draw the actual string where pvmem is pointing
   cpct_drawStringM1("Welcome to CPCtelera!", pvmem);

   // Loop forever
   while (1);
}
