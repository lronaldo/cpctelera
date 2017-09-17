//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "globaldata.h"

/////////////////////////////////////////////////////////////////////////
// updateKeyboardStatus
//    Checks user input and updates status of the relevant keys of 
// the keyboard
//
void updateKeyboardStatus() {
   TKey *k;    // Pointer to an element of the g_keys array
   u8    i;    // Counter

   // First read all present status of the keys in the keyboard
   cpct_scanKeyboard();

   // Then iterate through our array of keys to consider, and 
   // check if there has been status changes on any of them

   // Get a pointer to the first element of the keys array
   // That will be incremented in every loop with k++
   k = g_keys;
   for(i=0; i < G_NKEYS; i++, k++) {
      // Modifications depend in whether the key is pressed or not
      if (cpct_isKeyPressed(k->key)) {
         // Key is pressed now, check if that represents a change
         // with respect to previous status, and update status accordingly
         switch(k->status) {
            // If key was "just pressed" move it to Still pressed
            case KeySt_Pressed:  { k->status = KeySt_StillPressed; break; }
            // If key was free or released, move it to "just pressed"
            case KeySt_Free:
            case KeySt_Released: { k->status = KeySt_Pressed; }
         }
      } else {
         // Key is released now, check if that represents a change
         // with respect to previous status, and update status accordingly
         switch(k->status) {
            // If key was pressed in any form, move it to "just released"
            case KeySt_Pressed:
            case KeySt_StillPressed: { k->status = KeySt_Released; break; }
            // If key was already released, move it to Free
            case KeySt_Released:     { k->status = KeySt_Free; }
         }         
      }
   }
}