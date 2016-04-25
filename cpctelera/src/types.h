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
//-------------------------------------------------------------------------------

#ifndef CPCTELERA_TYPES_H
#define CPCTELERA_TYPES_H

///
/// Types: Aliases for builtin types
///
///		CPCtelera's short useful aliases for standard SDCC built-in types.
///
///		u8  - unsigned char      (u8  = unsigned  8-bits, 1 byte )
///		i8  - signed char        (i8  = integer   8-bits, 1 byte )
///		u16 - unsigned int       (u16 = unsigned 16-bits, 2 bytes)
///		i16 - signed int         (i16 = integer  16-bits, 2 bytes)
///		u32 - unsigned long      (u32 = unsigned 32-bits, 4 bytes)
///		i32 - signed long        (i32 = integer  32-bits, 4 bytes)
///		u64 - unsigned long long (u32 = unsigned 64-bits, 8 bytes)
///		i64 - signed long long   (i32 = integer  64-bits, 8 bytes)
///		f32 - float              (f32 = float    32-bits, 4 bytes)
///
typedef unsigned char       u8; 
typedef signed char         i8;
typedef unsigned int       u16;
typedef signed int         i16;
typedef unsigned long      u32;
typedef signed long        i32;
typedef unsigned long long u64;
typedef signed long long   i64;
typedef float              f32;

#endif
