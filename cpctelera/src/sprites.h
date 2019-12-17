//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
//### MODULE: Sprites                                               ###
//#####################################################################
//### This module contains several functions and routines to manage ###
//### sprites and video memory in an Amstrad CPC environment.       ###
//#####################################################################
//

#ifndef CPCT_SPRITES_H
#define CPCT_SPRITES_H

#include <types.h>
#include "sprite_types.h"
#include "pixel_macros.h"
#include "transparency_table_macros.h"
#include "drawToSpriteBuffer/drawToSpriteBuffer.h"
#include "screenToSprite/screenToSprite.h"
#include "drawTile/drawTile.h"
#include "flipping/flipping.h"
#include "blending/blending.h"
#include "clipping/clipping.h"

// Functions to transform firmware colours for a group of pixels into a byte in screen pixel format
extern   u8 cpct_px2byteM0 (u8 px0, u8 px1) __z88dk_callee;
extern   u8 cpct_px2byteM1 (u8 px0, u8 px1, u8 px2, u8 px3);

// Sprite and box drawing functions
extern void cpct_drawSolidBox        (void *memory, u8 colour_pattern, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSprite          (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteMasked    (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteMaskedAlignedTable(const void *psprite, void* pvideomem, 
                                              u8 height, u8 width, const void* pmasktable) __z88dk_callee;

#endif
