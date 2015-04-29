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
   // Disable firmware and set Mode 0
   cpct_disableFirmware();
   cpct_setVideoMode(0);

   // Lets Draw 1 sprite on page C0 (RAM bank 3) and 
   // ... 2 sprites on page 80 (RAM bank 2)
   cpct_drawSprite(G_newton_sprite, (void*)(0xC000 +  912), 16, 32);
   cpct_drawSprite(G_newton_sprite, (void*)(0x8000 +  662), 16, 32);
   cpct_drawSprite(G_newton_sprite, (void*)(0x8000 + 1240), 16, 32);

   // MAIN Loop
   while(1) {
      // Scan keyboard
      cpct_scanKeyboard_f();

      // Change Video Memory Page if we press Key 1 or Key 2
      if      (cpct_isKeyPressed(Key_1))
         cpct_setVideoMemoryPage(cpct_pageC0);
      else if (cpct_isKeyPressed(Key_2))
         cpct_setVideoMemoryPage(cpct_page80);
   }
}
