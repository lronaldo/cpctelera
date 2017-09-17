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
#include <string.h>

//
// MAIN PROGRAM:
//   Do not disable firmware in this example, as printf makes use of it through putchar
//
void main(void) {
   // Clear Screen filling it up with 0's
   cpct_clearScreen(0);

   // Print out some messages using printf and sizeof to know the sizes in bytes
   // ...of all the builting types in SDCC (using CPCtelera's aliases)
   // We use Firmware Screen Character Commands to change colour on screen.
   // Printing \0XX is equivalent to printing the character XX (in octal).
   // Character 15 (017 in octal) does the PEN command, and uses immediate next
   // character as the parameter for PEN. Then, \017\003 is equivalent to PEN 3.
   //
   printf("      \017\003Welcome to \017\002CPCtelera\017\003!\017\001\n\r\n\r");
   printf("This  example  makes  use  of standard C");
   printf("libraries  to  print out  byte sizes  of");
   printf("standard  SDCC  types, using \017\002CPCtelera\017\001's");
   printf("convenient aliases.\n\r\n\r");
   printf("Size of \017\003 u8 \017\001=\017\002 %d \017\001byte\n\r", sizeof(u8));
   printf("Size of \017\003u16 \017\001=\017\002 %d \017\001byte\n\r", sizeof(u16));
   printf("Size of \017\003u32 \017\001=\017\002 %d \017\001byte\n\r", sizeof(u32));
   printf("Size of \017\003u64 \017\001=\017\002 %d \017\001byte\n\r", sizeof(u64));
   printf("Size of \017\003 i8 \017\001=\017\002 %d \017\001byte\n\r", sizeof(i8));
   printf("Size of \017\003i16 \017\001=\017\002 %d \017\001byte\n\r", sizeof(i16));
   printf("Size of \017\003i32 \017\001=\017\002 %d \017\001byte\n\r", sizeof(i32));
   printf("Size of \017\003i64 \017\001=\017\002 %d \017\001byte\n\r", sizeof(i64));
   printf("Size of \017\003f32 \017\001=\017\002 %d \017\001byte\n\r", sizeof(f32));

   // Loop forever
   while (1);
}
