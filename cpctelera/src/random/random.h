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

//
// Title: Random
//
#ifndef RANDOM_H
#define RANDOM_H

//
// Uniform Random Generators
//
// Based on simple linear congruential algebra
extern u8   cpct_getRandomUniform_u8_f  (u8 entropy_byte) __z88dk_fastcall;
extern void cpct_setRandomSeedUniform_u8(u8      newseed) __z88dk_fastcall;

// Based on Galois Linear-Feedback Shift Register
#include "glfsr16taps.h"
extern void cpct_setSeed_glfsr16     (u16 newseed) __z88dk_fastcall;
extern void cpct_setTaps_glfsr16     (u16    taps) __z88dk_fastcall;
extern u8   cpct_getRandomu8_glfsr16 ();
extern u16  cpct_getRandomu16_glfsr16();

// Based on Marsaglia's XOR shift algorithm
extern u32  cpct_mxor32_seed;
extern u32  cpct_nextRandom_mxor_u32 (u32 seed) __z88dk_fastcall;
extern u8   cpct_getRandom_mxor_u8   ();

#endif