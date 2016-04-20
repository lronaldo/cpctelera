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

//
// Title: Random
//
#ifndef _CPCT_RANDOM_H_
#define _CPCT_RANDOM_H_

#include "random_types.h"

//
// Uniform Random Generators
//

// -- Based on simple linear congruential algebra
//
extern u8   cpct_getRandom_lcg_u8  (u8 entropy_byte) __z88dk_fastcall;
extern void cpct_setSeed_lcg_u8    (u8      newseed) __z88dk_fastcall;

// -- Based on Galois Linear-Feedback Shift Register
//
extern void cpct_setSeed_glfsr16      (u16       newseed) __z88dk_fastcall;
extern void cpct_setTaps_glfsr16      (GLFSR16_TAPS taps) __z88dk_fastcall;
extern u8   cpct_getRandom_glfsr16_u8 ();
extern u16  cpct_getRandom_glfsr16_u16();

// -- Based on Marsaglia's XOR-shift algorithm
//

// Calculating next 32-bits value in the sequence (seed)
extern u32  cpct_nextRandom_mxor_u32    (u32 seed) __z88dk_fastcall;
extern u32  cpct_nextRandom_mxor_u8     (u32 seed) __z88dk_fastcall;
extern u32  cpct_nextRandom_mxor532_u8  (u32 seed) __z88dk_fastcall;

// RNG direct user generators (return the random value)
extern u32  cpct_mxor32_seed;
extern u8   cpct_getRandom_mxor_u8      ();
extern u16  cpct_getRandom_mxor_u16     ();
extern u32  cpct_getRandom_mxor_u32     ();
extern void cpct_setSeed_mxor           (u32 newseed) __z88dk_fastcall;

// -- Based on Marsaglia's XOR-shift+ algorithm
//
extern u8   cpct_getRandom_xsp40_u8  ();
extern void cpct_setSeed_xsp40_u8    (u16 seed8, u32 seed32) __z88dk_callee;

///
/// Macros: Simple aliases for most common random generators
///
///   This macros are designed to simplify the user interface for generating
/// random numbers. Most of the time, a Marsaglia XOR-shift RNG is best
/// choice for generating random numbers
///
///  cpct_rand8  - returns a random  <u8> value ( 8-bits). It uses <cpct_getRandom_mxor_u8>.
///  cpct_rand16 - returns a random <u16> value (16-bits). It uses <cpct_getRandom_mxor_u16>.
///  cpct_rand32 - returns a random <u32> value (32-bits). It uses <cpct_getRandom_mxor_u32>.
///  cpct_rand   - alias for <cpct_rand8>
///  cpct_srand  - sets the seed for the random number generator (32 bits). It uses <cpct_setSeed_mxor>.
///
#define cpct_rand8  cpct_getRandom_mxor_u8
#define cpct_rand16 cpct_getRandom_mxor_u16
#define cpct_rand32 cpct_getRandom_mxor_u32
#define cpct_rand   cpct_rand8
#define cpct_srand  cpct_setSeed_mxor

#endif
