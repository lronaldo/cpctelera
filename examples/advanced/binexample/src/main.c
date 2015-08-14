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

// Character bin will be automatically converted
extern const u8 G_character[192];

const u8 g_palette[5] = { 
// HW Value  | FW Value | Colour name
//-------------------------------------
   0x14,  // |    0     | Black
   0x15,  // |    2     | Bright Blue
   0x13,  // |   20     | Bright Cyan
   0x16,  // |    9     | Green
   0x0E   // |   15     | Orange
};


void main(void) {
   cpct_disableFirmware();
   cpct_setVideoMode(0);
   cpct_setPalette(g_palette, 5);
   cpct_drawSprite(G_character, (u8*)0xC000, 8, 24);
   
   // Loop forever
   while (1);
}
