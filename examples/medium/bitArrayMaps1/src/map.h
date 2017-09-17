//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
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
#include <types.h>

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// PUBLIC FUNCTIONS
///   Declared here to let other modules use them
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void map_setBaseMem  (u8* mem_location);
 u8* map_getBaseMem  ();
  u8 map_getTile     (u8 x, u8 y);
void map_drawTile    (u8 x, u8 y);
void map_changeTile  (u8 x, u8 y);
void map_setTile     (u8 x, u8 y, u8 value);
void map_draw        ();
void map_clear       ();