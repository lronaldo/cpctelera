//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>

void main(void) {
   // Clear Screen
   cpct_memset((void*)0xC000, 0x4000, 0);

   cpct_drawSolidBox((void*)0xC235, 0x3C, 10, 20); 
   cpct_drawSolidBox((void*)0xC245, 0x92, 10, 20); 
   cpct_drawSolidBox((void*)0xC255, 0x14, 10, 20); 

   cpct_drawSolidBox((void*)0xC325, 0xAA, 10, 20); 
   cpct_drawSolidBox((void*)0xC335, 0xA0, 10, 20); 
   cpct_drawSolidBox((void*)0xC345, 0x0A, 10, 20); 

   cpct_drawSolidBox((void*)0xC415, 0x55, 10, 20); 
   cpct_drawSolidBox((void*)0xC425, 0x50, 10, 20); 
   cpct_drawSolidBox((void*)0xC435, 0x05, 10, 20); 
                     
   cpct_drawSolidBox((void*)0xC505, 0xFF, 10, 20); 
   cpct_drawSolidBox((void*)0xC515, 0xF0, 10, 20); 
   cpct_drawSolidBox((void*)0xC525, 0x0F, 10, 20); 
   
   // Loop forever
   while (1);
}
