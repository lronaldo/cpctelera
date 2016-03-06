//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//
// Macros: CPCT_-X-BITARRAY
//
//    A group of useful macros to declare and define bitarrays.
//
// C Definitions:
//    #define CPCT_6BITARRAY (*Name*, *Elems*)
//    #define CPCT_4BITARRAY (*Name*, *Elems*)
//    #define CPCT_2BITARRAY (*Name*, *Elems*)
//    #define CPCT_1BITARRAY (*Name*, *Elems*)
//
// Parameters (1 byte):
//  Name   - C identifier for the bitarray.
//  Elems  - Number of elements the array will have (total items)
//
// Details:
//    These macros make more comfortable the process of declaring and defining
// bitarrays. Any bitarray is a normal array of bytes in the end. However, it
// is usually accessed through CPCtelera's bitarray function to manage elements
// smaller than 1 byte. 
//
//	  When a bitarray is defined, a calculation has to be made to know how many
// bytes are required to store the total amount of X-bits elements it will contain.
// For instance, to store 8 2-bits elements, 16 bits are needed, so 2 bytes needs
// to be allocated. Therefore, in the next code segment, both sentences are equivalent:
// (start code)
//    	CPCT_2BITSARRAY(my_array, 8);	// This allocates 2 bytes (8 2-bits elements)
//		u8 my_array[2]; 				// This does exactly the same.
// (end code)
//
//    These 4 macros can be used either for defining and for declaring bitarrays.
// This example shows how to define and declare different bitarrays:
// (start code)
//		// Declaring an array (without defining it)
//      //	   This one is a 40 bits array (5 bytes, 10 4-bits elements)
//		extern CPCT_4BITSARRAY(an_array, 10); 
//
//		// Define and populate a constant array 
//		//		This other is a 16-bits array (2 bytes, 8 2-bits elements)
//		const CPCT_2BITSARRAY(other_array, 8) = {
//			0b00011011, 0b11100100	// Populate with 8 elements (0,1,2,3,3,2,1,0)
//		};
// (end code)
// 
#define CPCT_6BITARRAY(Name, Elems) u8 Name[ (((Elems)/4 + !!((Elems) % 4)) * 3) ]
#define CPCT_4BITARRAY(Name, Elems) u8 Name[ ((Elems)/2 + 1) ]
#define CPCT_2BITARRAY(Name, Elems) u8 Name[ ((Elems)/4 + 1) ]
#define CPCT_1BITARRAY(Name, Elems) u8 Name[ ((Elems)/8 + 1) ]

#define CPCT_ENCODE6BITS(A, B, C, D) ((A)<<2) | ((B)>>4), ((B)<<4) | ((C)>>2), ((C)<<6) | ((D)&0x3F)
#define CPCT_ENCODE4BITS(A, B)       ((A)<<4) | ((B)>>4)
#define CPCT_ENCODE2BITS(A, B, C, D) ((A)<<6) | (((B)<<4)&0x30) | (((C)<<2)&0x0C) | ((D)&0x03)

#endif