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

#ifndef _SPRITES_H_
#define _SPRITES_H_

#include <types.h>
#include "credits.h"    // File generated from credits.png. See Makefile and build_config.mk

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
///////////
/////////// PALETTE AND SPRITE DECLARATIONS (Defined in sprites.c)
///////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// Palette
extern const u8 G_palette[16];

// Main Character Sprites
extern const u8 G_EMRright[4*16];
extern const u8 G_EMRright2[2*16];
extern const u8 G_EMRright3[4*16];
extern const u8 G_EMRleft[4*16];
extern const u8 G_EMRleft2[2*16];
extern const u8 G_EMRleft3[4*16];
extern const u8 G_EMRjumpright1[4*8];
extern const u8 G_EMRjumpright2[4*8];
extern const u8 G_EMRjumpright3[4*8];
extern const u8 G_EMRjumpright4[4*8];
extern const u8 G_EMRjumpleft1[4*8];
extern const u8 G_EMRjumpleft2[4*8];
extern const u8 G_EMRjumpleft3[4*8];
extern const u8 G_EMRjumpleft4[4*8];
extern const u8 G_EMRhitright[4*16];
extern const u8 G_EMRhitleft[4*16];

#endif