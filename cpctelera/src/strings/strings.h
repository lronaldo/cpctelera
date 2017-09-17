//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#ifndef CPCT_CHARACTERS_H
#define CPCT_CHARACTERS_H

#include <types.h>

// Functions for drawing ROM Characters on Graphics Screen
extern void cpct_drawCharM0     (void* video_memory, u8 fg_pen, u8 bg_pen, u8 ascii);
extern void cpct_drawCharM1     (void* video_memory, u8 fg_pen, u8 bg_pen, u8 ascii);
extern void cpct_drawCharM1_f   (void* video_memory, u8 fg_pen, u8 bg_pen, u8 ascii);
extern void cpct_drawCharM2     (void* video_memory, u8 pen, u8 ascii);

// Functions for drawing Strings with ROM Characters on Graphics Sceen
extern void cpct_drawStringM0   (void* string, void* video_memory, u8 fg_pen, u8 bg_pen);
extern void cpct_drawStringM1   (void* string, void* video_memory, u8 fg_pen, u8 bg_pen);
extern void cpct_drawStringM1_f (void* string, void* video_memory, u8 fg_pen, u8 bg_pen);
extern void cpct_drawStringM2   (void* string, void* video_memory, u8 pen);

#endif
