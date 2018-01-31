//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2018 Arnaud Bouche
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
//### SUBMODULE: Screen To Sprite
//#####################################################################
//### This module contains specialized functions to read video memory
//### and copy it to linear arrays (sprites) in order to be used
//### later on as normal sprites.
//#####################################################################
//

#ifndef CPCT_SCREEN2SPRITE_H
#define CPCT_SCREEN2SPRITE_H

// Capture Video Memory area to buffer
extern void cpct_getScreenToSprite(u8* memory, u8* sprite, u8 width, u8 height) __z88dk_callee;

#endif