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
//### SUBMODULE: drawTile
//#####################################################################
//### This module contains specialized functions to draw tiles (small 
//### sprites normally used as background). Tiles are normally drawn 
//### aligned with respect to video memory layout.
//#####################################################################
//

#ifndef CPCT_DRAWTILE_H
#define CPCT_DRAWTILE_H

// Standard tile drawing functions
extern void cpct_drawTileAligned2x8    (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x8    (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned2x4_f  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned2x8_f  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x4_f  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x8_f  (void *sprite, void* memory) __z88dk_callee;

// Gray-code and Zig-Zag  tile drawing functions
extern void cpct_drawTileGrayCode2x8_af      (void *memory, void* sprite) __z88dk_callee;
extern void cpct_drawTileZigZagGrayCode4x8_af(void* memory, void* sprite) __z88dk_callee;

// Macros for standard names with suffix
#define cpct_drawTile2x8_a(SP, MEM)  cpct_drawTileAligned2x8  ((SP), (MEM))
#define cpct_drawTile2x8_af(SP, MEM) cpct_drawTileAligned2x8_f((SP), (MEM))
#define cpct_drawTile4x8_a(SP, MEM)  cpct_drawTileAligned4x8  ((SP), (MEM))
#define cpct_drawTile4x8_af(SP, MEM) cpct_drawTileAligned4x8_f((SP), (MEM))
#define cpct_drawTile2x4_af(SP, MEM) cpct_drawTileAligned4x4_f((SP), (MEM))
#define cpct_drawTile4x4_af(SP, MEM) cpct_drawTileAligned4x4_f((SP), (MEM))

#endif