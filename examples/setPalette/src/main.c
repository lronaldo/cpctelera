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

// Standard CPC Palette (16 colours for mode 0)
const unsigned char c_palette[16] = { 0x04, 0x0A, 0x13, 0x0C, 0x0B, 0x14, 0x15, 0x0D,
                                      0x06, 0x1E, 0x1F, 0x07, 0x12, 0x19, 0x0A, 0x07 };

// Keys used to modify individual palette colors
const cpct_keyID c_palkeys[16] = { Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7,
                                   Key_8, Key_9, Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Y };

// Fucntion that rotates the colors of the palette to the left
void rotatePalette(unsigned char* pal, unsigned char size) {
    unsigned char i, first;
    first = pal[0];
    for (i=0; i < size-1; i++)
        pal[i] = pal[i+1];
    pal[size-1] = first;
}

void main(void) {
   unsigned char x=0, y=0, t=0, k=0, border=1;
   char*dest = (char*)0xC000;
   unsigned char* palette = (unsigned char*)c_palette;

   cpct_disableFirmware();
   cpct_setVideoMode(0);
   cpct_setVideoPalette(palette, 16);

   while(1) {
      if (t) t--;

      cpct_scanKeyboardFast();
      if      (cpct_isKeyPressed(Key_CursorRight) && x < 64 ) { x++; dest++; }
      else if (cpct_isKeyPressed(Key_CursorLeft)  && x > 0  ) { x--; dest--; }
      if      (cpct_isKeyPressed(Key_CursorUp)    && y > 0  ) { dest -= (y-- & 7) ? 0x0800 : 0xC850; }
      else if (cpct_isKeyPressed(Key_CursorDown)  && y < 168) { dest += (++y & 7) ? 0x0800 : 0xC850; }
      if      (cpct_isKeyPressed(Key_Space) && !t) {
          rotatePalette(palette, 16);
          cpct_setVideoPalette(palette, 16);
          t=50;
      } else if (cpct_isKeyPressed(Key_Enter) && !t) {
          cpct_setVideoBorder(++border);
          t=25;
      } else {
         for (k=0; k<16; k++) {
            if (cpct_isKeyPressed(c_palkeys[k]) && !t) {
               palette[k] = (palette[k] + 1) & 0x1F;
               cpct_setVideoINK(k, palette[k]);
               t=25;
            }
         }
      }

      cpct_drawSprite(G_newton_sprite, dest, 16, 32);
   }
}
