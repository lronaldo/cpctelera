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

// Pointer to the memory location where screen video memory starts by default
#define SCR_VMEM  (u8*)0xC000

//
// MAIN: Keyboard check example
//
void main(void) {   
    // Disable firmware to prevent it from interfering with setVideoMode
    cpct_disableFirmware();

    // Set the border to a different color to ease visualizing
    cpct_setBorder(0);

    // Just draw some 16 bytes with a simple pattern on screen
    cpct_memset_f8(SCR_VMEM, 0xAA00, 16);

    // Infinite waiting loop
    while (1);
}
