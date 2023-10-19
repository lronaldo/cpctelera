//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2023 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
// Set up the Amstrad
//
void initialize_cpc() {
   cpct_disableFirmware();    // Disable firmware to prevent it from interfering with setPalette and setVideoMode
   cpct_setVideoMode(0);      // Set video mode 0 (160x200, 16 colours)
   cpct_setDrawCharM0(3, 0);  // Set PEN 3, PAPER 0 for Characters to be drawn using cpct_drawCharM0
}

//
// MAIN: Detecting Keypresses and printing ASCII values
//
void main(void) {
   initialize_cpc();    // Initialize the CPC

   // 
   // Infinite loop
   //
   while(1) {
      u8 ascii;   // We will need to store our ASCII value here

      // Scan Keyboard (fastest routine)
      // The Keyboard has to be scanned to obtain pressed / not pressed status of
      // every key before checking each individual key's status.
      cpct_scanKeyboard_f();

      // Obtain the ASCII value of the currently pressed Key
      ascii = cpct_getKeypressedAsASCII();
      if (ascii != 0) {                            // ascii == 0 means no key is pressed, so we check first
         cpct_drawCharM0(CPCT_VMEM_START, ascii);  // Some key is pressed, print its ascii value to the screen
      }
   } 
}
