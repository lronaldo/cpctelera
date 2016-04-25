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

// Funtion Pointer type, to point to memset functions that receive a pointer and 2 16-bit integers
typedef void (*TMemsetFunc)(void*, u16, u16) __z88dk_callee;

//
// Get a 16-bits colour pattern out of a sequence of 4 firmware colours (one for each mode 0 pixel)
//
u16 getColourPattern(u8 c1, u8 c2, u8 c3, u8 c4) {
   return 256*cpct_px2byteM0(c3, c4) + cpct_px2byteM0(c1, c2);
}

//
// Wait for the VSYNC n consecutive times (n/50 seconds wait, aprox)
//
void waitNVSYNCs(u8 n) {
   do {
      // Wait for next VSYNC is detected
      cpct_waitVSYNC();

      // If this is not the last VSYNC to wait, 
      // wait for 2 halts, to ensure VSYNC signal has stopped
      // before waiting to detect it again
      if (--n) {
         __asm__ ("halt"); // Halt stops the Z80 until next interrupt.
         __asm__ ("halt"); // There are 6 interrupts per VSYNC (1/300 seconds each)
      }
   } while(n);
}

//
// Clear 2 times the screen with a given colour to see the effect,
//    using standard cpct_memset function
//
void doSomeClears8(u8 colour, u8 vsyncs) {
   u8  i;
   for(i=0; i < 2 ; i++) {
      u8 pattern = cpct_px2byteM0(colour, colour);
      waitNVSYNCs(vsyncs);
      cpct_memset(CPCT_VMEM_START, pattern, 0x4000);
      waitNVSYNCs(vsyncs);
      cpct_memset(CPCT_VMEM_START,       0, 0x4000);
   }
}


//
// Clear 2 times the screen with a given colour to see the effect,
//    using a given memset function
//
void doSomeClears(TMemsetFunc func, u8 colour, u8 vsyncs) {
   u8  i;
   for(i=0; i < 2 ; i++) {
      u16 pattern = getColourPattern(colour, colour, colour, colour);
      waitNVSYNCs(vsyncs);
      func(CPCT_VMEM_START, pattern, 0x4000);
      waitNVSYNCs(vsyncs);
      func(CPCT_VMEM_START,       0, 0x4000);
   }
}

//
// MAIN: Loop infinitely while clearing the screen, to test it visually
//
void main(void) {   
   u8 colour = 1, vsyncs = 50;

   // Disable firmware to prevent it from interfering with setVideoMode
   // Then set videomode to 0
   cpct_disableFirmware(); 
   cpct_setVideoMode(0);

   // Infinite screen clearing loop
   while(1) {
      // Clear the screen using standard memset (5,17 frames to clear)
      cpct_setBorder(4);
      doSomeClears8(colour, vsyncs);

      // Clear the screen using fast  8-byte chuncks memset (1,77 frames to clear)
      cpct_setBorder(1);
      doSomeClears(&cpct_memset_f8,  colour, vsyncs);

      // Clear the screen using fast 64-byte chuncks memset (1,33 frames to clear)
      cpct_setBorder(5);
      doSomeClears(&cpct_memset_f64, colour, vsyncs);

      // Next colour and less time
      if (++colour > 15) colour = 1;
      if (! --vsyncs) vsyncs = 50;
   }
}
