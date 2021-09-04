//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2018 Arnaud Bouche (Arnaud6128)
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
//### MODULE: Sprites
//### SUBMODULE: ColorReplace
//#####################################################################
//### This module contains routines to replace colors in sprites
//#####################################################################
//

#ifndef CPCT_COLORREPLACE_H
#define CPCT_COLORREPLACE_H

// Functions to replace a color in sprite M0
extern void cpct_spriteColourizeM0         (u16 rplcPat, u16 size, void* sprite) __z88dk_callee;
extern void cpct_spriteMaskedColourizeM0   (u16 rplcPat, u16 size, void* sprite) __z88dk_callee;

// Functions to replace a color in sprite M1
extern void cpct_spriteColourizeM1         (u16 rplcPat, u16 size, void* sprite) __z88dk_callee;
extern void cpct_spriteMaskedColourizeM1   (u16 rplcPat, u16 size, void* sprite) __z88dk_callee;

// Functions to draw a sprite M0 with a color replaced 
extern void cpct_drawSpriteColorizeM0(u8* sprite, u8* destMem, u8 width, u8 height, u16 rplcPat) __z88dk_callee;
extern void cpct_drawSpriteMaskedColorizeM0(u8* sprite, u8* destMem, u8 width, u8 height, u16 rplcPat) __z88dk_callee;
extern void cpct_drawSpriteMaskedAlignedColorizeM0(u8* sprite, u8* destMem, u8 width, u8 height, const u8* maskTable, u16 rplcPat) __z88dk_callee;

// Functions to draw a sprite M1 with a color replaced 
extern void cpct_drawSpriteColorizeM1(u8* sprite, u8* destMem, u8 width, u8 height, u8 oldColor, u8 newColor) __z88dk_callee;
extern void cpct_drawSpriteMaskedColorizeM1(u8* sprite, u8* destMem, u8 width, u8 height, u8 oldColor, u8 newColor) __z88dk_callee;
extern void cpct_drawSpriteMaskedAlignedColorizeM1(u8* sprite, u8* destMem, u8 width, u8 height, u8 oldColor, u8 newColor, const u8* maskTable) __z88dk_callee;

#endif
