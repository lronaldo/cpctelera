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

#include <cpctelera_all.h>

void main(void) {
    char sprite_pressed[16] = {
        0x00, 0x00,
        0x10, 0x10,
        0x20, 0x20,
        0x03, 0x03,
        0x04, 0x04,
        0x50, 0x50,
        0x60, 0x60,
        0x07, 0x07
    };
    char sprite_notpressed[16] = {
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00
    };
    char *sprite = sprite_notpressed;

    cpct_disableFirmware();
    cpct_setVideoMode(0);
    while (1) {
        cpct_scanKeyboard();

        if (cpct_isKeyPressed(0x8005))  // Matrix Line 5, bit 7 => Space
            sprite = sprite_pressed;
        else
            sprite = sprite_notpressed;

        cpct_drawSprite2x8_aligned(sprite, (char*)0xC000);
    }
}
