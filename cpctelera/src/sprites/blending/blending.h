//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
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
//-------------------------------------------------------------------------------

//#####################################################################
//### MODULE: Sprites
//### SUBMODULE: blending
//#####################################################################
//### This module contains routines to draw sprites in special ways to
//### blend them with backgrounds in screen video memory
//#####################################################################
//

#ifndef CPCT_BLENDSPRITES_H
#define CPCT_BLENDSPRITES_H

// Blended Draw Functions
extern void cpct_drawSpriteBlended   (void *memory, u8 height, u8 width, void *sprite) __z88dk_callee;

// Functions to modify behaviour of other functions
extern void cpct_setBlendMode (CPCT_BlendMode mode) __z88dk_fastcall;

#endif