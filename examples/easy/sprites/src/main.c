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

void main(void) {
   u8  x=10, y=10;
   u8* pvideomem;

   cpct_disableFirmware();
   cpct_fw2hw(G_palette, 4);
   cpct_setPalette(G_palette, 4);
   cpct_setVideoMode(1);

   while(1) {
      cpct_scanKeyboard_f();
//      if      (cpct_isKeyPressed(Key_CursorRight) && x < 68 ) { x++; dest++; }
//      else if (cpct_isKeyPressed(Key_CursorLeft)  && x > 0  ) { x--; dest--; }
//      if      (cpct_isKeyPressed(Key_CursorUp)    && y > 0  ) { dest -= (y-- & 7) ? 0x0800 : 0xC850; }
//      else if (cpct_isKeyPressed(Key_CursorDown)  && y < 138) { dest += (++y & 7) ? 0x0800 : 0xC850; }
      if      (cpct_isKeyPressed(Key_CursorRight) && x < 68 ) ++x; 
      else if (cpct_isKeyPressed(Key_CursorLeft)  && x > 0  ) --x; 
      if      (cpct_isKeyPressed(Key_CursorUp)    && y > 0  ) --y;
      else if (cpct_isKeyPressed(Key_CursorDown)  && y < 138) ++y;
      
      pvideomem = cpct_getScreenPtr((u8*)0xC000, x, y);
      cpct_drawSprite(G_LCT1, pvideomem, 12, 62);
   }
}
