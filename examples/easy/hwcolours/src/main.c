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
#include <stdio.h>

//
// MAIN PROGRAM:
//   Do not disable firmware in this example, as printf makes use of it through putchar
//
void main(void) {
   u8 i;
   
   // Clear Screen filling it up with 0's
   cpct_clearScreen(0);

   // Print out a table with the firmware colours and their equivalent
   // Hardware colour values using cpct_getHWColour

   // We use Firmware Screen Character Commands to change colour on screen.
   // Printing \0XX is equivalent to printing the character XX (in octal).
   // Character 15 (017 in octal) does the PEN command, and uses immediate next
   // character as the parameter for PEN. Then, \017\003 is equivalent to PEN 3.
   //
   printf("        \017\003Hardware Colour values\017\002          ");
   printf("This example shows the  equivalence between firmware co-");
   printf("lour  values  and harwdware  colour values. \017\003CPCtelera\017\002's ");
   printf("functions that change colours use hardware ones.\n\r\n\r");
   printf("   \017\003==================================\n\r");
   printf("   || \017\002FIRM -- HARD \017\003|| \017\002FIRM -- HARD \017\003||\n\r");
   printf("   ==================================\n\r");
   for (i=0; i < 13; ++i) {
      printf("   \017\003||  \017\001%2d  \017\003--  \017\001%2d\017\003  ",       i, cpct_getHWColour(i));
      printf("\017\003||  \017\001%2d  \017\003--  \017\001%2d\017\003  ||\n\r", i+13, cpct_getHWColour(i+13));
   }
   printf("   ==================================\n\r");

   // Loop forever
   while (1);
}
