//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#define  HW_BACKBUFFER (u8*)(0x8000)

u8* video_buffer;

u8* video_getBackBuffer() {
   return video_buffer;
}

void video_switchBuffers() {
   // Change Video Buffer
   if (video_buffer == HW_BACKBUFFER) {
      cpct_setVideoMemoryPage(cpct_page80);
      video_buffer = CPCT_VMEM_START;
   } else {
      cpct_setVideoMemoryPage(cpct_pageC0);
      video_buffer = HW_BACKBUFFER;
   }
}

void video_initBuffers() {
   cpct_memset(HW_BACKBUFFER, 0, 0x4000);  
   video_switchBuffers();
}
