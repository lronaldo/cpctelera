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

//#####################################################################
//### MODULE: Memutils                                              ###
//#####################################################################
//### Utilities to manage memory blocks                             ###
//#####################################################################
//

#ifndef CPCT_MEMUTILS_H
#define CPCT_MEMUTILS_H

#include <types.h>
#include "relocation.h"

// Standard memory management functions
extern void cpct_memset    (void *array, u8  value, u16 size) __z88dk_callee;
extern void cpct_memset_f8 (void *array, u16 value, u16 size) __z88dk_callee;
extern void cpct_memset_f64(void *array, u16 value, u16 size) __z88dk_callee;
extern void cpct_memcpy    (void *to, const void *from, u16 size) __z88dk_callee;

// Stack manipulation
extern void cpct_setStackLocation(void *memory) __z88dk_fastcall;

// Macro to check conditions at compile time and issue errors
#define BUILD_BUG_ON(condition) ((void)sizeof(char[2 - 2*!!(condition)]))

#endif
