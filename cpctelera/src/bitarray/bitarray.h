//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 Alberto García García
//  Copyright (C) 2015 Pablo Martínez González
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
#include "bitarray_macros.h"

// Get bit functions
extern   u8 cpct_getBit  (void *array, u16 pos) __z88dk_callee;
extern   u8 cpct_get2Bits(void *array, u16 pos) __z88dk_callee;
extern   u8 cpct_get4Bits(void *array, u16 pos) __z88dk_callee;
extern   u8 cpct_get6Bits(void *array, u16 pos) __z88dk_callee;

// Set bit functions
extern void cpct_setBit  (void *array, u16 value, u16 pos) __z88dk_callee;
extern void cpct_set2Bits(void *array, u16 value, u16 pos) __z88dk_callee;
extern void cpct_set4Bits(void *array, u16 value, u16 pos) __z88dk_callee;
extern void cpct_set6Bits(void *array, u16 value, u16 pos) __z88dk_callee;

#endif