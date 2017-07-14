//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//#####################################################################
//### MODULE: Sprites                                               ###
//#####################################################################
//### This module contains several functions and routines to manage ###
//### sprites and video memory in an Amstrad CPC environment.       ###
//#####################################################################
//

//
// Title: Sprite Macros&Constants
//

#ifndef CPCT_SPRITES_H
#define CPCT_SPRITES_H

#include <types.h>

// Functions to transform firmware colours for a group of pixels into a byte in screen pixel format
extern   u8 cpct_px2byteM0 (u8 px0, u8 px1);
extern   u8 cpct_px2byteM1 (u8 px0, u8 px1, u8 px2, u8 px3);

// Tile drawing functions
extern void cpct_drawTileAligned2x8    (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x8    (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned2x4_f  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned2x8_f  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileGrayCode2x8_af(void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x4_f  (void *sprite, void* memory) __z88dk_callee;
extern void cpct_drawTileAligned4x8_f  (void *sprite, void* memory) __z88dk_callee;

// Sprite and box drawing functions
extern void cpct_drawSprite          (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSpriteMasked    (void *sprite, void* memory, u8 width, u8 height) __z88dk_callee;
extern void cpct_drawSolidBox        (void *memory, u8 colour_pattern, u8 width, u8 height);

//
// Macro: cpct_memPage6
//
// 	Macro that encodes a video memory page in the 6 Least Significant bits (LSb)
// of a byte, required as parameter for <cpct_setVideoMemoryPage>
//
// C Definition:
//	#define <cpct_memPage6> (*PAGE*)
//
// Parameters (1 byte):
//	(1B) PAGE - Video memory page wanted 
//
// Returns:
//	 u8	- Video Memory Page encoded in the 6 LSb of the byte.
//
// Details:
//	 This is just a macro that shifts *PAGE* 2 bits to the right, to leave it
// with just 6 significant bits. For more information, check functions
// <cpct_setVideoMemoryPage> and <cpct_setVideoMemoryOffset>.
//
#define cpct_memPage6(PAGE) ((PAGE) >> 2)

//
// Constants: Video Memory Pages
//
// Useful constants defining some typical Video Memory Pages to be used as 
// parameters for <cpct_setVideoMemoryPage>
//
//	cpct_pageCO - Video Memory Page 0xC0 (0xC0··)
//	cpct_page8O - Video Memory Page 0x80 (0x80··)
//	cpct_page4O - Video Memory Page 0x40 (0x40··)
//	cpct_page0O - Video Memory Page 0x00 (0x00··)
//
#define cpct_pageC0 0x30
#define cpct_page80 0x20
#define cpct_page40 0x10
#define cpct_page00 0x00

#endif
