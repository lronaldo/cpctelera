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

// Declare RGB Palette
extern const u8 G_rgb_palette[17][3];

// Declare table to convert from RGB to CPC hardware colour values
extern const u8 G_rgb2hw[3][3][3];

// Declare palette functions (defined in palette.c)
void setPALColourRGBLimited(u8 pal_index, const u8 rgb[3], const u8 maxrgb[3]);
void fade_in (const u8 rgb[][3], u8 min_pi, u8 max_pi, u8 wait);
void fade_out(const u8 rgb[][3], u8 min_pi, u8 max_pi, u8 wait);
void setBlackPalette(u8 min_pi, u8 max_pi);
