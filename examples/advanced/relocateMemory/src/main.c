//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
//------------------------------------------------------------------------------
// Explanation:
//
// This example shows how to use the macros CPCT_ABSOLUTE_LOCATION_AREA and
// CPCT_RELOCATABLE_AREA. In this case, some routines and data are required only 
// when doing CPC initializations at the start of the program. Therefore, 
// the strategy followed is:
//    1. Code starts at 0x8000 (Z80CODELOC=0x8000 in cfg/build_config.mk). That
// tells the compiler to place all code and data by default from 0x8000 to 0xFFFF.
// Compiler and linker decide how to place this code (in a consecutive way from
// 0x8000 onwards).
//    2. The part of the code that is only required once is manually placed at
// 0x4000 using CPCT_ABSOLUTE_LOCATION_AREA. 
//    3. At the start of the program, initialization routines are called and
// executed (they are placed at 0x4000). This routines set up the main video
// buffer (0xC000-0xFFFF) with a green background colour and a message in the 
// middle. This is what the user will see at the start of the program.
//    4. A call to setUpVideoBuffer routine fills up the second video buffer
// (0x4000-0x7FFF) with a colour pattern for background and another message
// in the middle. This overwrites everything previously placed at 0x4000 
// (the initialization routines) assuming it is not required anymore. 
//    5. Program continues.
//
// By doing this, some bytes of memory space can be reused. They store code
// and data until being used, and then are reused as memory video buffer.
//------------------------------------------------------------------------------

#include <cpctelera.h>

// We will use main video memory (at 0xC000) and a double buffer (at 0x4000)
#define VMEM_0       CPCT_VMEM_START
#define VMEM_1       (u8*)0x4000
#define VMEM_SIZE    0x4000

////////////////////////////////////////////////////////////////////////////////
// Sets a video memory buffer with a background colour and a string
// drawn in the middle of the screen
//
void setUpVideoBuffer(u8* vmem, u16 c_pattern, u8* string, u8 pen, u8 bpen) {
   cpct_memset_f64(vmem, c_pattern, VMEM_SIZE);
   vmem = cpct_getScreenPtr(vmem, 0, 80);
   cpct_setDrawCharM0(pen, bpen);
   cpct_drawStringM0(string, vmem);
}

//=============================================================================
//=============================================================================
// Next code area is placed at 0x4000 in memory
//=============================================================================
//=============================================================================
CPCT_ABSOLUTE_LOCATION_AREA(0x4000);

// Color palette to use. We only require this to be stored until 
// cpct_setPalette is called. After that, this 16 bytes could be overwritten
// with anything else. Therefore, we placed them at 0x4000.
const u8 g_palette[16] = { 0x1A, 0x03, 0x01, 0x00, 0x0D, 0x19, 0x14, 0x12, 
                           0x16, 0x15, 0x13, 0x06, 0x07, 0x08, 0x02, 0x0A };


//////////////////////////////////////////////////////////////////////////////
// Initialization of the CPC
//    This function will be called once at the start of the program. After
// that, it won't be required anymore. Therefore, its code will be placed at
// 0x4000 to be used once and then removed when an image is written to 
// this second video buffer (0x4000-0x7FFF)
///
void initializeCPC() {
   cpct_disableFirmware();          // Disable the firmware not to interfere with us
   cpct_setVideoMode(0);            // Set mode 0 (160x200, 16 colours)
   cpct_setPalette(g_palette, 16);  // Set colour palette
   cpct_setBorder(g_palette[0]);    // Set the border with same colour used for background (0)
   
   // Set up the main video buffer (0xC000) with a message and all colours set up
   setUpVideoBuffer(VMEM_0, 0, "Main Screen Buffer", 6, 0);
}

//=============================================================================
//=============================================================================
// Area placed at 0x4000 ends here.
// Next code area is flagged relocatable. Compiler will place it continuing
// the main area that starts at 0x8000
//=============================================================================
//=============================================================================

CPCT_RELOCATABLE_AREA();

//////////////////////////////////////////////////////////////////////////////
// Waits until a new key is pressed, ignoring previous maintained keys
//
void wait4KeyPress () {
   // Wait while previous keys are still being pressed
   do { cpct_scanKeyboard(); } while (  cpct_isAnyKeyPressed() );

   // Wait for a new keypress 
   do { cpct_scanKeyboard(); } while ( !cpct_isAnyKeyPressed() );
}

// Global array containing pages for both screen buffers 
// (0xC000 => pageC0, 0x4000 => page40)
const u8 g_buffers[2] = { cpct_pageC0, cpct_page40 };

//////////////////////////////////////////////////////////////////////////////
// MAIN: code execution starts here
//    Initializes buffers at 0x4000 and 0xC000 and goes switching between
// both of them on keypresses
//
void main(void) {
   u8  scr = 0;      // Video buffer selector: selects what is to be shown on the screen
   u16 pattern;      // Will contain a 4 pixel-colour pattern 
   
   // First, Initialize the CPC
   initializeCPC();

   // Construct a color pattern with 4 consecutive pixels of 4 different
   // colours in mode 0, that will be used to fill up the second buffer
   pattern = ( cpct_px2byteM0(4, 6) << 8 ) | cpct_px2byteM0(8, 9);

   // Second, Set up the 0x4000 video buffer. This set up will erase 
   // all code placed in the 0x4000 area that has already been used
   // for initializing the CPC. Therefore, this code will have been 
   // used one and then replaced to gain some space.
   setUpVideoBuffer(VMEM_1, pattern, "0x4000 Screen Buffer", 0, 6);

   // Main Loop
   while (1) {
      // Wait for VSYNC and show pressently 
      // selected video buffer in the screen
      cpct_waitVSYNC();
      cpct_setVideoMemoryPage(g_buffers[scr]);  // Sets the memory page that will be shown on screen
      
      // Select the alternate video buffer and wait
      // for a keypress to change what is being shown on screen
      scr = scr ^ 0x01;    // Does operation scr XOR 1, which alternates the value of the last bit of scr, 
                           // ... so that it alternates between 0 and 1.
      wait4KeyPress();
   }
}
