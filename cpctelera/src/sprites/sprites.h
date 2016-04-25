//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "transparency_table_macros.h"

// Functions to transform firmware colours for a group of pixels into a byte in screen pixel format
extern   u8 cpct_px2byteM0 (u8 px0, u8 px1) __z88dk_callee;
extern   u8 cpct_px2byteM1 (u8 px0, u8 px1, u8 px2, u8 px3);

// Tile drawing functions
extern void cpct_drawTileAligned2x8  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x8  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned2x4_f(void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned2x8_f(void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x4_f(void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x8_f(void *sprite, void* memory) __z88dk_callee;

// Sprite and box drawing functions
extern void cpct_drawSprite          (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteMasked    (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteBlended   (void *memory, u8 height, u8 width, void *sprite) __z88dk_callee;
extern void cpct_drawSolidBox        (void *memory, u8 colour_pattern, u8 width, u8 height);
extern void cpct_drawSpriteMaskedAlignedTable(const void *psprite, void* pvideomem, 
                                              u8 width, u8 height, const void* pmasktable) __z88dk_callee;

// Sprite flipping functions
extern void cpct_hflipSpriteM0   (u8 width, u8 height, void* sprite) __z88dk_callee;
extern void cpct_hflipSpriteM1   (u8 width, u8 height, void* sprite) __z88dk_callee;
extern void cpct_hflipSpriteM2   (u8 width, u8 height, void* sprite) __z88dk_callee;

// Sprite flipping functions (ROM-friendly versions)
extern void cpct_hflipSpriteM0_r (void* sprite, u8 width, u8 height) __z88dk_callee;
extern void cpct_hflipSpriteM1_r (void* sprite, u8 width, u8 height) __z88dk_callee;
extern void cpct_hflipSpriteM2_r (void* sprite, u8 width, u8 height) __z88dk_callee;

// Masked Sprite flipping functions
extern void cpct_hflipSpriteMaskedM0(u8 width, u8 height, void* sprite) __z88dk_callee;
extern void cpct_hflipSpriteMaskedM1(u8 width, u8 height, void* sprite) __z88dk_callee;
extern void cpct_hflipSpriteMaskedM2(u8 width, u8 height, void* sprite) __z88dk_callee;

// Functions to modify behaviour of other functions
extern void cpct_setBlendMode (CPCT_BlendMode mode) __z88dk_fastcall;

#endif
