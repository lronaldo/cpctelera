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

#ifndef CPCTELERA_TYPES_H
#define CPCTELERA_TYPES_H

///
/// Short useful aliases for standard SDCC built-in types
///  * u = unsigned
///  * i = integer (signed)
///  * f = float
///  * Type is followed by number of bits (8, 16, 32, 64)
///
typedef unsigned char       u8; 
typedef char                i8;
typedef unsigned int       u16;
typedef int                i16;
typedef unsigned long      u32;
typedef long               i32;
typedef unsigned long long u64;
typedef long long          i64;
typedef float              f32;

#endif
