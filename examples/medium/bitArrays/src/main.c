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
#include <cpctelera.h>

// TFunc
//  3 enumerated aliases for the 3 getBits functions available on CPCtelera
//
typedef enum {
   f_getbit,    // Function to get bits one by one
   f_get2bits,  // Function to get pairs of bits
   f_get4bits   // Function to get chunks of 4 bits
} TFunc;

//
// printArray
//   This function prints the contents of a bitarray as characters on the screen.
// Value 0 is shown as '_' for visual clarity. Other values are shown as characters
// [1, 2, 3, 4, 5, 6, 7, 8, 9, :, ;, <, =, >, ?].
//
void printArray(u8* pvideomem, void *array, u8 nItems, TFunc thefunction) {
    u8 out = 0; // Value returned by getBit functions [0-15]
    u8 i;       // Counter through the number of elements in the array
    u8 c;       // Character to draw 

    // Iterate through all the items in the array to print them one by one
    for (i = 0; i < nItems; ++i) {

      // Access the array using the function that has been told to us 
      // in the parameter 'thefunction'
      switch(thefunction) {
        // Get element as a single bit (As return value could be anything, 
        // we distinguish between 0 and >0, so out will finally be 0 or 1).
        case f_getbit:   out = (cpct_getBit (array, i) > 0); break;

        // Get element as a pair of bits 
        case f_get2bits: out = cpct_get2Bits(array, i); break;

        // Get element as a chunk of 4 bits
        case f_get4bits: out = cpct_get4Bits(array, i); break;
      }
      // Depending on the value got from getXBits function, calculate
      // the character we have to print. (0 = '_', >0 = '0' + out)
      if (out) 
        c = '0' + out;
      else
        c = '_';

      // Draw the character and point to the next byte in memory (next location
      // to draw a character, as 1 byte = 8 pixels in mode 2)
      cpct_drawCharM2(pvideomem, c);
      pvideomem++;
    }
}

//
// Bit Arrays Example: Main
//
void main (void) {
  u8 i, j;       // Counters for loops
  CPCT_1BITARRAY(array1, 80); // Array of 80 1-bit elements (10 bytes)
  CPCT_2BITARRAY(array2, 80); // Array of 80 2-bit elements (20 bytes)
  CPCT_4BITARRAY(array4, 80); // Array of 80 4-bit elements (40 bytes)

  // Disable firmware to prevent it from restoring video mode or
  // interfering with our drawChar functions
  cpct_disableFirmware();

  // Set mode 2 for visual clarity on arrays printed
  cpct_setVideoMode(2);
  cpct_setDrawCharM2(1, 0); // Draw characters in Foreground colour

  // 
  // Main Loop: loop forever showing arrays
  //
  while(1) {
    // First, erase all contents of our 3 arrays,
    // setting all their bits to 0
    cpct_memset(array1, 0, 10);
    cpct_memset(array2, 0, 20);
    cpct_memset(array4, 0, 40);

    //
    // Test 1: Set to 1 each bit on the array1 individually (all others to 0)
    //
    for (i = 0; i < 80; ++i) {
      // Set Bit i to 1
      cpct_setBit(array1, 1, i);

      // Print the complete array at the top of the screen
      printArray(CPCT_VMEM_START, array1, 80, f_getbit); 
      
      // Reset again the bit to 0 an iterate
      cpct_setBit(array1, 0, i);
    }

    //
    // Test 2: Fill in the array2 with individual values from 3 to 1 
    //              (all the rest should be 0)
    //
    for (j = 3; j > 0; --j) { 
      for (i = 0; i < 80; ++i) {
        // Set the index i to the value j (1 to 3)
        cpct_set2Bits(array2, j, i);

        // Print the complete array
        printArray((u8*)0xC0A0, array2, 80, f_get2bits);

        // Reset the value of the item to 0 again
        cpct_set2Bits(array2, 0, i);
      }
    }

    //
    // Test 3: Fill in the array4 with consecutive elements from 0 to 15,
    //         16 times, rotating all the 16 elements through all the positions
    //         in the array.
    //
    for (j = 0; j < 16; j++) { 
      for (i = 0; i < 80; ++i) {
        // Increment value using loop indexes and calculate module 16 (AND 0x0F)
        u8 value = (i + j) & 0x0F;

        // Set next 4-bits element (i) to the calculated value and print the array
        cpct_set4Bits(array4, value, i);
        printArray((u8*)0xC140, array4, 80, f_get4bits);
      }
    }

    //
    // Test 4: Fill the array1 with 1's
    //
    for (i = 0; i < 80; ++i) {
      // Set next bit i to 1  
      cpct_setBit(array1, 1, i);

      // Print the complete array1 again
      printArray(CPCT_VMEM_START, array1, 80, f_getbit); 
    }

    //
    // Test 5: Fill the array2 with 3's, then with 2's and then with 1's
    //
    for (j = 3; j > 0; --j) { 
      for (i = 0; i < 80; ++i) {
        // Set next bit i to j (3, 2, 1)  
        cpct_set2Bits(array2, j, i);

        // Print the complete array again
        printArray((u8*)0xC0A0, array2, 80, f_get2bits); 
      }
    }
  }
}