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

#include <strings/strings.h>
#include "message.h"

//
// Draw message (if any) on the screen
//
TMessage g_message;
void drawMessage() {
   // Draw message only if there is time to draw it
   if (g_message.time > 1) {
      // Draw the message
      cpct_setDrawCharM0(1, 0);
      cpct_drawStringM0(g_message.str, g_message.videopos);
      g_message.time--;
   } else if (g_message.time > 0) {
      // Clean the message
      cpct_setDrawCharM0(0, 0);
      cpct_drawStringM0(g_message.str, g_message.videopos);
      g_message.time=0;
   }
}

//
// Copy a string into a given buffer
//
void strcpy(i8* to, const i8* from){
   while (*to++ = *from++);
}

// 
// Inserts a number (3 digits at most) at a given position in a string
// (adds \0 at the end)
//
void concatNum (i8* to, i8 num) {
   i8 digits[5] = { 32, 48, 48, 48, 0 };
   u8 d, unum;

   // Check for the sign
   if (num < 0) {
      unum = -num;
      digits[0]=45;
   } else {
      unum = num;
   }

   // Calculate the ascii values of the 3 digits, dividing the number
   for (d=3; d != 0; --d) {
      u8 r=unum % 10;
      unum /= 10;
      digits[d]=48 + r;
   }

   // Insert the characters at the given position of the string
   // (digits[4]=0, contains '\0' to left the string null terminated)
   for (d=0; d<5; d++){
      *to++ = digits[d];
   }
}
