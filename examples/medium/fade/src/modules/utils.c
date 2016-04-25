//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//  Copyright (C) 2015 SunWay / Fremos / Carlio 
//  Copyright (C) 2015 Dardalorth / Fremos / Carlio
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

#include "utils.h"

///////////////////////////////////////////////////////////////////
// Wait n complete screen frames of (1/50)s
//
void wait_frames(u16 nframes) {
   u16 i, j;   // frame counter and active wait loop counter

   // Loop for nframe times, waiting for VSYNC
   for (i=0; i < nframes; i++) {
      cpct_waitVSYNC();

      // VSYNC is usually active for ~1500 cycles, then we have to do 
      // something that takes approximately this amount of time before
      // waiting for the next VSYNC, or we will find the same VSYNC signal
      // This active wait loop will do at least 750 comparisons, what
      // is the same as 750*4 cycles (at least)
      for (j=0; j < 750; j++);
   }
}
