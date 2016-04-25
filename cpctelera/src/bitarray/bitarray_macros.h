//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#ifndef CPCT_BITARRAY_MACROS_H
#define CPCT_BITARRAY_MACROS_H

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// File: Useful macros
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Group: Bitarray definition and declaration
////////////////////////////////////////////////////////////////////////
//    Group of macros to declare and/or define bitarrays.
//
// Macro: CPCT_1BITARRAY
//    Define or declare arrays with 1-bit elements
//
// Macro: CPCT_2BITARRAY
//    Define or declare arrays with 2-bits elements
//
// Macro: CPCT_4BITARRAY
//    Define or declare arrays with 4-bits elements
// 
// Macro: CPCT_6BITARRAY
//    Define or declare arrays with 6-bits elements
//
// Parameters (1 byte):
//    Name   - C identifier for the bitarray.
//    Elems  - Number of elements the array will have (total items)
//
// C Definitions:
//    #define CPCT_6BITARRAY (*Name*, *Elems*)
//
//    #define CPCT_4BITARRAY (*Name*, *Elems*)
//
//    #define CPCT_2BITARRAY (*Name*, *Elems*)
//
//    #define CPCT_1BITARRAY (*Name*, *Elems*)
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

////////////////////////////////////////////////////////////////////////
// Group: Bitarray element encoding
////////////////////////////////////////////////////////////////////////
//    Macros that help initializing, populating and inserting elements 
// manually into different types of bitarrays
//
// Macro: CPCT_ENCODE6BITS 
//    Encodes 4 6-bits elements into 3 bytes for a <CPCT_6BITARRAY>
// 
// Macro: CPCT_ENCODE4BITS
//    Encodes 2 4-bits elements into 1 byte for a <CPCT_4BITARRAY>
//
// Macro: CPCT_ENCODE2BITS
//    Encodes 4 2-bits elements into 1 byte for a <CPCT_2BITARRAY>
//
// C Definitions:
//   #define CPCT_ENCODE6BITS(A, B, C, D)
//
//   #define CPCT_ENCODE4BITS(A, B)
//
//   #define CPCT_ENCODE2BITS(A, B, C, D)
//
// Parameters (2-4 bytes):
//  A-D - Individual elements to be inserted in the bitarray
//
// Parameter Restrictions:
//    CPCT_ENCODE6BITS - Each value must be in the range [0, 63]
//    CPCT_ENCODE4BITS - Each value must be in the range [0, 15]
//    CPCT_ENCODE2BITS - Each value must be in the range [0, 3]
//
// Details:
//    These macros help in the initialization of bitarray elements mainly, and
// can also help on their population and modification. They get individual elements
// (one number for each element) and pack them together, putting each element into
// its corresponding bits.
//
//    Let us understand this better with an example. Suppose we wanted to initialize a 
// <CPCT_4BITARRAY> with these elements: 5, 10, 15, 1. We could do it manually 
// this way:
// (start code)
//    // Define and populate a new bitarray with 4-bits elements
//    CPCT_4BITARRAY(mybitarray, 4) = { 
//       0x5A, // First 2 elements of the bitarray, 5 and 10, 4-bits each, hexadecimally codified
//       0xF1  // Next 2 elements of the bitarray, 15 and 1, 4-bits each, hexadecimally codified
//    };
// (end code)
//    In this case, we know that each hexadecimal digit corresponds to 4-bits and 
// use this to define 4 elements in 2 bytes, being each one a hexadecimal digit. 
// We can do exactly the same, easily, using <CPCT_ENCODE4BITS> macro:
// (start code)
//    // Define and populate a new bitarray with 4-bits elements
//    CPCT_4BITARRAY(mybitarray, 4) = { 
//       CPCT_ENCODE4BITS( 5, 10),  // Add elements 5 and 10
//       CPCT_ENCODE4BITS(15,  1)   // Add elements 15 and 1
//    };
// (end code)
//    This code does the same as previous one, but its easier to do. We use <CPCT_ENCODE4BITS>
// macro and directly write desired values in order, as in a conventional array definition. 
// This let us forget about internal codification and focus on our data.
//
// Examples of use:
//    This example is from a platform game, where user controls a running man that 
// has to jump over metal spikes. The game needs an array defining the layout of 
// the floor, having 4 types of floor block: (0) normal floor, (1) decelerator floor,
// (2) jump-booster floor, (3) spikes. This is the definition of the first level:
// (start code)
//    // Define level 1 layout. It starts with normal floor, going then through 
//    // some easy jumps and ending with a decelerator floor previous to a 
//    // jump-booster which is required for the final jump.
//    CPCT_2BITARRAY(level_1, 24) = {
//         CPCT_ENCODE2BITS(0, 0, 0, 0) // Start with normal floor
//       , CPCT_ENCODE2BITS(3, 0, 0, 0) // Some easy jumps (spike and then 3 floors)
//       , CPCT_ENCODE2BITS(3, 0, 0, 0) 
//       , CPCT_ENCODE2BITS(1, 1, 2, 2) // Decelerator floor, followed by 2 jump-boosters
//       , CPCT_ENCODE2BITS(3, 3, 3, 3) // Big spike hole
//       , CPCT_ENCODE2BITS(0, 0, 0, 0) // Normal floor at the end of the level
//    }
// (end code)
//    The same operation this code does (defining and populating an array), could have
// been manually done this way:
// (start code)
//    // Defining and populating level 1, using 2 bits for each element
//    // (4 elements per byte, so 6 bytes required, as 6 x 4 = 24). We 
//    // use binary numbers for clarity (2bits per element), and include hexadecimal
//    // and decimal for comparison purposes only.
//    level_1[6] = { 0b00000000, 0b11000000, 0b11000000, 0b01011010, 0b11111111, 0b00000000 };
//    //         = { 0x00, 0xC0, 0xC0, 0x5A, 0xFF, 0x00 }; Same in hexadecimal
//    //         = {    0,  192,  192,   80,  255,    0 }; Same in decimal
// (end code)
//
//    Another example could be the definition of a height map for a 2D lateral game
// filled with mountains. Each element of the next array will refer to the height
// of the floor at a given location. Thinking of a floor that will never be taller
// than 63 pixels, we can use a 6-bits array to define the layout of the map. 
// This would be a map for the second level of this game:
// (start code)
//    // Define the height map for the level 2 that     #        < 28
//    // will be a great mountain like this one >>     ###       < 24
//    CPCT_6BITARRAY(heightmap_l2, 12) = {      //     ###       < 20
//         CPCT_ENCODE6BITS( 0,  4,  8, 16)     //    ######     < 16
//       , CPCT_ENCODE6BITS(24, 28, 24, 16)     //    ######     < 12
//       , CPCT_ENCODE6BITS(16,  4,  0,  0)     //   #######     <  8
//    };                                        // _#########__  <  4
// (end code)
// 
#define CPCT_ENCODE6BITS(A, B, C, D) ((A)<<2) | ((B)>>4), ((B)<<4) | ((C)>>2), ((C)<<6) | ((D)&0x3F)
#define CPCT_ENCODE4BITS(A, B)       ((A)<<4) | ((B)>>4)
#define CPCT_ENCODE2BITS(A, B, C, D) ((A)<<6) | (((B)<<4)&0x30) | (((C)<<2)&0x0C) | ((D)&0x03)

#endif