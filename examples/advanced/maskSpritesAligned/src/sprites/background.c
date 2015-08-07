//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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

#include "sprites.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BACKGROUND TILEMAP DEFINITION (Created with tiled)
// 
const u8 g_background[40*50] = {
   // CSV file created exporting tilemap directly from tiled and adding final commas
   #include "map.csv"
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TILESET DEFINITION (Generated with cpct_img2tileset)
// 
const u8* const g_tileset [25] = {
   g_tile0, g_tile1, g_tile2, g_tile3, g_tile4, g_tile5, g_tile6, g_tile7, g_tile8, 
   g_tile9, g_tile10, g_tile11, g_tile12, g_tile13, g_tile14, g_tile15, g_tile16, 
   g_tile17, g_tile18, g_tile19, g_tile20, g_tile21, g_tile22, g_tile23, g_tile24
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TILE DEFINITIONS (Generated with cpct_img2tileset)
//
const u8 g_tile0[2*4] = {
   0x00, 0x00,
   0x00, 0xC0,
   0xC0, 0xC0,
   0xC0, 0xC0
};

const u8 g_tile1[2*4] = {
   0xC0, 0xC0,
   0xC0, 0xC0,
   0xC0, 0xC0,
   0xC0, 0xC0
};

const u8 g_tile2[2*4] = {
   0x00, 0x00,
   0xC0, 0x00,
   0xC0, 0xC0,
   0xC0, 0xC0
};

const u8 g_tile3[2*4] = {
   0x00, 0x00,
   0xC0, 0xC0,
   0xC0, 0xC0,
   0xC0, 0xC0
};

const u8 g_tile4[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0xC0, 0xC0,
   0xC0, 0xC0
};

const u8 g_tile5[2*4] = {
   0x0C, 0x0C,
   0x08, 0x04,
   0x08, 0x04,
   0x0C, 0x0C
};

const u8 g_tile6[2*4] = {
   0x0C, 0x0C,
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x00
};

const u8 g_tile7[2*4] = {
   0x0C, 0x0C,
   0x08, 0x04,
   0x0C, 0x0C,
   0x0C, 0x0C
};

const u8 g_tile8[2*4] = {
   0x0C, 0x0C,
   0x08, 0x04,
   0x08, 0x0C,
   0x0C, 0x0C
};

const u8 g_tile9[2*4] = {
   0x0C, 0x0C,
   0x0C, 0x04,
   0x08, 0x04,
   0x0C, 0x0C
};

const u8 g_tile10[2*4] = {
   0x0C, 0x0C,
   0x0C, 0x0C,
   0x0C, 0x0C,
   0x0C, 0x0C
};

const u8 g_tile11[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x00
};

const u8 g_tile12[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0x80, 0x80,
   0xC0, 0xC0
};

const u8 g_tile13[2*4] = {
   0x04, 0x00,
   0x00, 0x00,
   0x08, 0x04,
   0x00, 0x00
};

const u8 g_tile14[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x04
};

const u8 g_tile15[2*4] = {
   0x00, 0x40,
   0x00, 0xC0,
   0x40, 0xC4,
   0x40, 0xC0
};

const u8 g_tile16[2*4] = {
   0x80, 0x00,
   0xC0, 0x80,
   0xC0, 0xC0,
   0xC0, 0x80
};

const u8 g_tile17[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x40
};

const u8 g_tile18[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0x80, 0x00,
   0x80, 0x00
};

const u8 g_tile19[2*4] = {
   0x00, 0x00,
   0x00, 0x00,
   0x00, 0x08,
   0x00, 0x00
};

const u8 g_tile20[2*4] = {
   0x00, 0x04,
   0x00, 0x04,
   0x80, 0x84,
   0xC0, 0x84
};

const u8 g_tile21[2*4] = {
   0xC0, 0x00,
   0x08, 0x00,
   0x08, 0x80,
   0x48, 0xC0
};

const u8 g_tile22[2*4] = {
   0x40, 0x40,
   0xC0, 0xC0,
   0x40, 0xC0,
   0xC0, 0xC0
};

const u8 g_tile23[2*4] = {
   0xC0, 0x80,
   0xC8, 0x00,
   0xC0, 0x80,
   0xC0, 0x80
};

const u8 g_tile24[2*4] = {
   0x00, 0x00,
   0x04, 0x00,
   0x00, 0x00,
   0x00, 0x00
};


