//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "sprites.h"

//
// MAIN: Keyboard check example
//
void main(void) {
    u8 space_pressed = 0;     // Status of the space key (0 not pressed, 1 pressed)

    // Initialize screen
    //
    // Disable firmware to prevent it from interfering with setVideoMode
    cpct_disableFirmware();
    // Set video mode 0: 160x200, 16 colours
    cpct_setVideoMode(0);     

    // Infinite loop
    //
    while (1) {
        // Scan the keyboard to fill up cpct_keyboardStatusBuffer with
        // the status of the keys and joystiks (pressed / not pressed)
        cpct_scanKeyboard();

        if (!space_pressed && cpct_isKeyPressed(Key_Space)) {
            cpct_drawSprite(G_newton_sprite, (u8*)0xC000, 16, 32);
            space_pressed = 1;
        } else if (space_pressed && !cpct_isKeyPressed(Key_Space)) {
            cpct_drawSolidBox((u8*)0xC000, 0x00, 16, 32);
            space_pressed = 0;
        }
    }
}
