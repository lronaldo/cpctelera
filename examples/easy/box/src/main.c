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

//
// Details about this example:
//
//    cpct_px2byteM1 function gets 4 values, corresponding to 4 firmware colours, 
// for each one of the 4 pixels that can be drawn with 1 single byte in Mode 1. 
// Then this function mixes all these 4 values into 1 8-bit value which is in 
// screen pixel format. Drawing 1 byte of that pattern is the same as drawing 4
// pixels on the screen with the 4 given colours, in the given order. 
//
//    This byte pattern can be used in drawSolidBox and clearScreen functions.
//

//
// Creating Boxes example: main
//
void main(void) {
   // Clear the screen (fill it with 0's)
   cpct_clearScreen(0x00);

   // Lets draw some boxes

   // 3 boxes with varying colour patterns
   cpct_drawSolidBox((u8*)0xC235, cpct_px2byteM1(2, 2, 1, 1), 10, 20); 
   cpct_drawSolidBox((u8*)0xC245, cpct_px2byteM1(1, 0, 2, 1), 10, 20); 
   cpct_drawSolidBox((u8*)0xC255, cpct_px2byteM1(0, 2, 0, 1), 10, 20); 

   // 3 stripped boxes in 2 alternating colours
   cpct_drawSolidBox((u8*)0xC325, 0xAA, 10, 20); // 0xAA = cpct_px2byteM1(3, 0, 3, 0)
   cpct_drawSolidBox((u8*)0xC335, 0xA0, 10, 20); // 0xA0 = cpct_px2byteM1(1, 0, 1, 0)
   cpct_drawSolidBox((u8*)0xC345, 0x0A, 10, 20); // 0x0A = cpct_px2byteM1(2, 0, 2, 0)

   // Another 3 stripped boxes, with the strips displaced
   cpct_drawSolidBox((u8*)0xC415, 0x55, 10, 20); // 0x55 = cpct_px2byteM1(0, 3, 0, 3)
   cpct_drawSolidBox((u8*)0xC425, 0x50, 10, 20); // 0x50 = cpct_px2byteM1(0, 1, 0, 1)
   cpct_drawSolidBox((u8*)0xC435, 0x05, 10, 20); // 0x05 = cpct_px2byteM1(0, 2, 0, 2)
                     
   // 3 Boxes in solid colour (4 pixels of the same colour)
   cpct_drawSolidBox((u8*)0xC505, cpct_px2byteM1(3, 3, 3, 3), 10, 20); // 0xFF 
   cpct_drawSolidBox((u8*)0xC515, cpct_px2byteM1(2, 2, 2, 2), 10, 20); // 0xF0
   cpct_drawSolidBox((u8*)0xC525, cpct_px2byteM1(1, 1, 1, 1), 10, 20); // 0x0F
   
   // After drawing, loop forever
   while (1);
}
