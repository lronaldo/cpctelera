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
//------------------------------------------------------------------------------

#include <types.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  Random uniform distribution
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Index that controls the next random number to get from the distribution
u8 g_nextRand;

// Random uniform distribution repeating every 2^8
const u8 g_randUnif[256] = {253, 69, 158, 112, 225, 82, 36, 35, 105, 42, 
108, 176, 219, 199, 94, 24, 255, 215, 241, 72, 8, 175, 32, 248, 192, 115, 
91, 44, 213, 80, 130, 63, 70, 217, 93, 205, 96, 102, 13, 243, 109, 134, 
159, 210, 66, 231, 184, 128, 81, 56, 170, 182, 221, 99, 78, 122, 147, 117, 
148, 23, 118, 250, 220, 90, 216, 34, 188, 111, 207, 43, 208, 181, 26, 190, 
119, 139, 218, 4, 150, 164, 146, 186, 77, 162, 71, 46, 168, 84, 123, 238, 
83, 239, 171, 67, 142, 58, 136, 41, 226, 61, 212, 187, 251, 116, 33, 86, 
6, 138, 174, 143, 98, 97, 110, 76, 29, 120, 135, 137, 145, 12, 154, 149, 
64, 18, 124, 7, 59, 235, 113, 19, 242, 79, 10, 60, 240, 101, 3, 100, 106, 
2, 252, 197, 1, 21, 92, 152, 151, 47, 132, 249, 51, 22, 114, 191, 27, 246,
201, 125, 55, 144, 88, 39, 20, 157, 53, 165, 194, 195, 232, 233, 17, 49,
183, 103, 203, 172, 127, 45, 126, 68, 166, 237, 167, 198, 11, 230, 173, 34, 
244, 245, 196, 200, 95, 206, 224, 73, 227, 236, 57, 211, 25, 121, 38, 161, 
202, 131, 189, 48, 153, 133, 204, 129, 5, 31, 156, 65, 50, 54, 247, 74, 
160, 107, 223, 140, 179, 222, 254, 178, 9, 180, 163, 40, 214, 229, 15, 193,
228, 28, 52, 177, 87, 37, 89, 185, 155, 14, 209, 16, 169, 104, 0, 141, 85, 
62, 30, 75};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Get a random uniform number 
//    inc: increment to add to the index of the next random number to get
//
u8 getRandomUniform(u8 inc) {
   g_nextRand += inc;               // Move to next random value to get
   return g_randUnif[g_nextRand];   // Return the random value
}