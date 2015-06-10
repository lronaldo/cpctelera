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
//-------------------------------------------------------------------------------

//
// Title: Tilemaps
//
//


#ifndef CPCT_TILEMAPS_H
#define CPCT_TILEMAPS_H

#include <types.h>

// Setting global configuration for tilemanagement
extern void cpct_setTileset         (void* tileset);
extern void cpct_setGetTileFunction (void* getTileFunction);

// Drawing tile rows
extern void cpct_drawTileRowAligned4x8(void* pvideomem, void* tileset, void* tilearray, u8 arraysize);

#endif
