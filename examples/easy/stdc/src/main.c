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
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include <stdio.h>
#include <string.h>

//
// MAIN PROGRAM:
//   Do not disable firmware in this example, as printf makes use of it through putchar
//
void main(void) {
   // Clear Screen
   memset((void*)0xC000, 0, 0x4000);

   // Print out some messages using printf and sizeof
   printf("Welcome to CPCtelera!\n\r\n\r");
   printf("This example makes use of standard C libraries ");
   printf("to print out byte sizes of standard SDCC types.\n\r\n\r");
   printf("Size of  u8=%d\n\r", sizeof(u8));
   printf("Size of u16=%d\n\r", sizeof(u16));
   printf("Size of u32=%d\n\r", sizeof(u32));
   printf("Size of u64=%d\n\r", sizeof(u64));
   printf("Size of  i8=%d\n\r", sizeof(i8));
   printf("Size of i16=%d\n\r", sizeof(i16));
   printf("Size of i32=%d\n\r", sizeof(i32));
   printf("Size of i64=%d\n\r", sizeof(i64));
   printf("Size of f32=%d\n\r", sizeof(f32));

   // Loop forever
   while (1);
}
