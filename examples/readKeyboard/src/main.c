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
#include "newton_sprite.def"

void main(void) {
    const unsigned char *sprite = G_void_sprite;

    cpct_disableFirmware();
    cpct_setVideoMode(0);
    while (1) {
        cpct_scanKeyboardFast();

        if (cpct_isKeyPressed(Key_Space))
            sprite = G_newton_sprite;
        else
            sprite = G_void_sprite;

        cpct_drawSprite(sprite, (void*)0xC000, 16, 32);
    }
}
