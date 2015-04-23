//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 Alberto García García
//  Copyright (C) 2015 Pablo Martínez González
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
//######################################################################
//### MODULE: Bit Array                                              ###
//### Developed by Alberto García García and Pablo Martínez González ###
//### Reviewed and optimized by ronaldo / Cheesetea                  ###
//######################################################################
//### This module contains functions to get and set groups of 1, 2   ###
//### and 4 bit in a char array. So data in arrays can be compressed ###
//### in a transparent way to the programmer.                        ###
//######################################################################
//
#ifndef CPCT_BITARRAY_H
#define CPCT_BITARRAY_H

#include <types.h>

///
/// Function Declarations
///
extern   u8 cpct_getBit  (void *array, u16 pos);
extern   u8 cpct_get2Bits(void *array, u16 pos);
extern   u8 cpct_get4Bits(void *array, u16 pos);

extern void cpct_setBit  (void *array, u16 pos, u8 value);
extern void cpct_set2Bits(void *array, u16 pos, u8 value);
extern void cpct_set4Bits(void *array, u16 pos, u8 value);

#endif