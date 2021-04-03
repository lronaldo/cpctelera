//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
//                COMPRESSION / DECOMPRESSION EXAMPLE
//------------------------------------------------------------------------------
// This program decompresses an image directly into video memory, using three
// different compression methods.
//------------------------------------------------------------------------------
// Folder 'img/' contains file 'savage_colors.png'. This image has a resolution
// of 160x200 pixels and 16 colours, as it will be displayed as a full screen
// mode 0 image.
//
// Using configuration file 'cfg/image_conversion.mk', this image is
// converted to screen format, and stored as a binary file in folder
// 'img/screenformat/'.
//
// With the configuration file 'cfg/compression.mk' this binary file is compressed
// using three different compression methods: ZX0, ZX0B (ZX0 backwards), and
// ZX7B (ZX7 backwards). This compression generates in 'src/compressed' folder
// a '.c' and '.h' file for each compression algorithm. Of course, for each
// different compression method used we'll need a different decompressor.
//
// For ZX0 decompression, we need to provide cpct_zx0_decrunch function the
// memory adress where compressed data starts and the memory address where
// uncompressed data starts. From there it continues decompressing until the
// end of compressed data is reached and the end of the uncompressed data
// is written to memory.
//
// On the other hand, ZX7B and ZX0B methods start decompression reading
// compressed data from its end, and start writing at the end of the
// uncompressed destination area. Then decompression continues backwards until
// the start of compressed data is reached and the start of uncompressed data
// is written. Therefore we'll need to provide cpct_zx7b_decrunch_s and
// cpct_zx0b_decrunch functions the end of uncompressed destination area and
// the end of compressed data.
//
// As in this example we are uncompressing the whole video memory area, the
// start of the uncompressed destination area is 0xC000, and the end of the
// uncompressed destination area is 0xFFFF.
//------------------------------------------------------------------------------

// include CPCtelera's constants and definitions
#include <cpctelera.h>

// include files generated with cfg/compression.mk
#include "compressed/data_zx0.h"
#include "compressed/data_zx0b.h"
#include "compressed/data_zx7b.h"

// include palette generated with cfg/image_conversion.mk
#include "savagecolors_palette.h"

// By default, video memory starts at adcress 0xC000 and ends at address 0xFFFF.
// CPCT_VMEM_START is already defined in CPCtelera.
#define VIDEO_MEMORY_END   (void*)(0xFFFF)


void main(void) {
  // Pointer to video memory
  u8* pvmem;
  // Disable firmware: we dont want it to interfere with our code
  cpct_disableFirmware();
  // Change to Mode 0 (160x200, 16 colours)
  cpct_setVideoMode(0);
  // Set up the palette using hardware colour values
  cpct_setPalette(savagecolors_palette,16);
  // Make pvmem point to the byte in video memory where we want to
  // print our string (top left corner is at the start of video memory)
  pvmem = CPCT_VMEM_START;

   // Repeat next part forever
  while (1) {
    // Clear the screen filling it up with 0's
    cpct_clearScreen_f64(0);
    // Decompress pack data_zx7b using ZX7 backwards algorithm, so we need
    // to use as parameters for cpct_zx7b_decrunch_s the end of the
    // destination area and the end of the compressed data
    cpct_zx7b_decrunch_s (VIDEO_MEMORY_END, data_zx7b_end);
    // Draw a string with the name of the decruncher function used
    cpct_drawStringM0("cpct_zx7b_decrunch_s", pvmem);
    // Wait for a keyboard press
    do {cpct_scanKeyboard();} while (!cpct_isAnyKeyPressed());

    // Clear the screen filling it up with 0's
    cpct_clearScreen_f64(0);
    // Decompress pack data_zx0 using ZX0 algorithm, so we need to
    // use as parameters for cpct_zx0_decrunch the start of the
    // destination area and the start of the compressed data
    cpct_zx0_decrunch (CPCT_VMEM_START, data_zx0);
    // Draw a string with the name of the decruncher function used
    cpct_drawStringM0("cpct_zx0_decrunch", pvmem);
    // Wait for a keyboard press
    do {cpct_scanKeyboard();} while (!cpct_isAnyKeyPressed());

    // Clear the screen filling it up with 0's
    cpct_clearScreen_f64(0);
    // Decompress pack data_zx0b using ZX0 backwards algorithm, so we need
    // to use as parameters for cpct_zx0b_decrunch the end of the
    // destination area and the end of the compressed data
    cpct_zx0b_decrunch (VIDEO_MEMORY_END, data_zx0b_end);
    // Draw a string with the name of the decruncher function used
    cpct_drawStringM0("cpct_zx0b_decrunch", pvmem);
    // Wait for a keyboard press
    do {cpct_scanKeyboard();} while (!cpct_isAnyKeyPressed());
  }
}
