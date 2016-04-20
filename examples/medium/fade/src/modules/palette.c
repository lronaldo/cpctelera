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

#include <cpctelera.h>
#include "palette.h"
#include "utils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Palette definition (Same palette for all the sprites)
//

// Palette for mode 0 coded in RGB using 3 nibbles (0x_RGB) 
// and 2 bits per nibble (0 = 0, 1 = 128, 2 = 255).
//  > Examples, 0x0111 means R=1, G=1, B=1 (128, 128, 128),
//  >           0x0201 means R=2, G=0, B=1 (255,   0, 128)
const u8 G_rgb_palette[17][3] = {
//    RGB          Firmware
   { 1, 1, 1 }, //  0x0D
   { 2, 1, 0 }, //  0x0F
   { 1, 0, 0 }, //  0x03
   { 2, 2, 0 }, //  0x18
   { 0, 0, 1 }, //  0x01
   { 0, 0, 2 }, //  0x14
   { 2, 0, 0 }, //  0x06
   { 2, 2, 2 }, //  0x1A
   { 0, 0, 0 }, //  0x00
   { 0, 0, 2 }, //  0x02
   { 0, 1, 2 }, //  0x0B
   { 0, 2, 0 }, //  0x12
   { 2, 0, 2 }, //  0x08
   { 1, 0, 2 }, //  0x05
   { 2, 1, 1 }, //  0x10
   { 0, 1, 0 }, //  0x09
   { 1, 1, 1 }  //  0x0D // Border
};

//////////////////////////////////////////////////////////////////////////////////////////
// Translation table from RGB to Amstrad CPC Hardware colour values
//    So, G_rgb2hw[R][G][B] = CPC hw colour
//    Indexes are R, G, B values with this mapping: 0=0, 1=128, 2=255 
//
const u8 G_rgb2hw[3][3][3] = {
   // R=0
   {  { 0x14, 0x04, 0x15 },    // R=0, G=0, B=(0, 1, 2)
      { 0x16, 0x06, 0x17 },    // R=0, G=1, B=(0, 1, 2)
      { 0x12, 0x02, 0x13 }  }, // R=0, G=2, B=(0, 1, 2)
   // R=1 (128)
   {  { 0x1C, 0x18, 0x1D },    // R=1, G=0, B=(0, 1, 2)
      { 0x1E, 0x00, 0x1F },    // R=1, G=1, B=(0, 1, 2)
      { 0x1A, 0x19, 0x1B }  }, // R=1, G=2, B=(0, 1, 2)
   // R=2 (255)
   {  { 0x0C, 0x05, 0x0D },    // R=2, G=0, B=(0, 1, 2)
      { 0x0E, 0x07, 0x0F },    // R=2, G=1, B=(0, 1, 2)
      { 0x0A, 0x03, 0x0B }  }  // R=2, G=2, B=(0, 1, 2)
};


///////////////////////////////////////////////////////////////////
// Sets a Palette colour from RGB values (0=0, 1=128, 2=255)
// but with a maximum limited value for each component (maxrgb)
//  It uses G_rgb2hw array to map RGB values into hardware colour values
//
void setPALColourRGBLimited(u8 pal_index, const u8 rgb[3], const u8 maxrgb[3]) {
  u8 i;     // Index to iterate the 3 R,G,B components
  u8 s[3];  // Final RGB values that will be set
  
  // Check the 3 RGB components individually, and truncate them
  // to left them below the limits established by maxrgb[]
  for (i=0; i < 3; ++i) {
    s[i] = rgb[i];  // The colour component to set is the one given in rgb[]

    // If this colour component exceeds the maximum value for this 
    // component, we truncate it to the maximum value
    if ( rgb[i] > maxrgb[i] ) 
      s[i] = maxrgb[i];
  }

  // Get the hardware colour value from its RGB components
  i = G_rgb2hw[ s[0] ][ s[1] ][ s[2] ];

  // Set the palette colour
  cpct_setPALColour(pal_index, i);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//// PALETTE FUNCTIONS
////    * Fade in
////    * Fade out
////    * Set all the colours to black
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Fade in palette effect 
//   Does a fade in palette effect applied to a [min_pi, max_pi]
// range of palette colours 
//
void fade_in(const u8 rgb[][3], u8 min_pi, u8 max_pi, u8 wait) {
  u8 col;               // Index of the colour component to change (R=0, G=1, B=2)
  u8 pi;                // Palette index of the colour being changed
  u8 maxrgb[3]={0,0,0}; // Maximum components for the 3 RGB values, initially 0 (to slowly increase)

  // Do the Fade in effect iteratively for all the 3 RGB components (R, G, B)
  for(col=0; col <= 2; ++col){

    // Go increasing the present maximum colour component up to 
    // its maximum value, 2 (0=0, 1=128, 2=255)
    while (maxrgb[col] <= 2) {
      
      // Set all the palette colours in the selected range again, 
      // with the present maxrgb[] limits
      for(pi=min_pi; pi <= max_pi; ++pi)
        setPALColourRGBLimited(pi, rgb[pi], maxrgb);

      ++maxrgb[col];     // Increase present maximum colour component limit
      wait_frames(wait); // Wait some frames to slow down the effect and see the changes
    }
  }
}


///////////////////////////////////////////////////////////////////
// Fade out palette effect 
//   Does a fade in palette effect applied to a [min_pi, max_pi]
// range of palette colours 
//
void fade_out(const u8 rgb[][3], u8 min_pi, u8 max_pi, u8 wait) {
  u8 col;               // Index of the colour component to change (R=0, G=1, B=2)
  u8 pi;                // Palette index of the colour being changed
  u8 maxrgb[3]={2,2,2}; // Maximum components for the 3 RGB values, initially 2 (to slowly decrease)

  // Do the Fade out effect iteratively for all the 3 RGB components (R, G, B)
  for(col=0; col <= 2; ++col){

    // Go decreasing the present maximum colour component down to 
    // its minimum value, 0 (2=255, 1=128, 0=0)
    do {
      --maxrgb[col];  // Decrease present maximum colour component limit

      // Set all the palette colours in the selected range again, 
      // with the present maxrgb[] limits
      for(pi=min_pi; pi <= max_pi; ++pi)
        setPALColourRGBLimited(pi, rgb[pi], maxrgb);

      wait_frames(wait); // Wait some frames to slow down the effect and see the changes
    } while (maxrgb[col] > 0);
  }
}

//////////////////////////////////////////////////////////////////
// Set all palette colours in a given range [min_pi, max_pi] 
// to black colour value (RGB = 0,0,0)
//
void setBlackPalette(u8 min_pi, u8 max_pi) {
  u8 i;

  // Go through all the palette colours in the range [min_pi, max_pi] 
  for(i=min_pi; i <= max_pi; ++i)
    cpct_setPALColour(i, G_rgb2hw[0][0][0]);  
}
