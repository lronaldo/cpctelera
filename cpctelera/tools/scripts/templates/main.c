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
   u8* video_memory_start  = (u8*)0xC000;
   u8  character_line_size = 0x050;

   // Clear Screen
   cpct_memset(video_memory_start, 0, 0x4000);

   // Draw String on the middle of the screen
   cpct_drawStringM1("Welcome to CPCtelera!",
                     video_memory_start + character_line_size * 12,
                     1, 0);
   // Loop forever
   while (1);
}
