//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2019 Arnaud Bouche (@Arnaud6128)
//  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//#####################################################################
//### MODULE: Sprites
//### SUBMODULE: clip
//#####################################################################
//### This module contains routines to draw partially sprites
//#####################################################################
//

#ifndef CPCT_CLIPSPRITES_H
#define CPCT_CLIPSPRITES_H

extern void cpct_drawSpriteClipped(u8 width_to_draw, const u8* sprite, u8* memory, u8 width, u8 height) __z88dk_callee;

#endif
