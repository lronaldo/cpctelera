//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
   unsigned int scr[2] = { 0xC000, 0x8000 };
   cpct_keyID   key[2] = { Key_1,  Key_2  };

   cpct_disableFirmware();
   cpct_setVideoMode(0);

   // Lets Draw 1 Sprite on Buffer 0 and 2 sprites on Buffer 1
   cpct_drawSprite(G_newton_sprite, (void*)(scr[0] +  912), 16, 32);
   cpct_drawSprite(G_newton_sprite, (void*)(scr[1] +  662), 16, 32);
   cpct_drawSprite(G_newton_sprite, (void*)(scr[1] + 1240), 16, 32);

   while(1) {
      unsigned char i;
      cpct_scanKeyboardFast();

      for (i=0; i < 2; i++) {
         if (cpct_isKeyPressed(key[i]))
            cpct_setVideoMemoryPage(cpct_memPage6(scr[i] >> 8));
      }
   }
}
