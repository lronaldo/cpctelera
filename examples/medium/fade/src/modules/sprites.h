//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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

#include <types.h>

//
// TSprite structure to group the pixel data of a sprite
// with its width,height values and x,y screen byte coordinates
// to locate it on the screen
//
typedef struct {
  const u8* sprite;  // Pointer to sprite pixel data 
  u8  x, y;          // Screen byte coordinates
  u8  w, h;          // Width and Height of the sprite in bytes
} TSprite;

// Declare sprites to be used
//  Goku sprite, by Dardalorth / Fremos / Carlio
extern const u8 G_Goku[980];
extern const u8 G_Vegeta[2880];
extern const u8 G_No13[2880];

