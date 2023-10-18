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
#include "sprites/character.h"

//////////////////////////////////////////////////////////////////////////////////////////
// DATA STRUCTURES AND CONSTANT VALUES
//

// Constant values
#define SCR_WIDTH_PIXELS      160
#define SCR_HEIGHT_PIXELS     200
#define PIXELS_PER_BYTE         2
#define SPRITE_WIDTH_PIXELS    16
#define SPRITE_HEIGHT_PIXELS   24
#define SPRITE_WIDTH_BYTES    SPRITE_WIDTH_PIXELS / PIXELS_PER_BYTE
#define SPRITE_HEIGHT_BYTES   SPRITE_HEIGHT_PIXELS

// Struct to note the shifting status of a sprite
typedef enum {
   ON_EVEN_PIXEL = 0,  // Sprite is un-shifted (starts on even pixel)
   ON_ODD_PIXEL  = 1   // Sprite is shifted    (starts on odd pixel)
} TShiftStatus;

// Struct for storing entity information
typedef struct {
   u8  x,  y;          // Pixel Location
   u8 nx, ny;          // Next pixel location
   u8  w,  h;          // Width and height of the entity (in bytes!)
   u8* sprite;         // Sprite
   TShiftStatus shift; // Sprite shifting status (EVEN, ODD)
} TEntity;

//////////////////////////////////////////////////////////////////////////////////////////
// Shift all pixels of a sprite to the right
//
void shiftSpritePixelsRight(u8* sprite, u8 size) {
   u8 prev_rightpixel,
      rightpixel;       // Values of the right-pixel of a byte, and the right-pixel of the previous byte

   // Shift all bits to the right, to move sprite 1 pixel to the right    
   prev_rightpixel = 0;
   do {
      // Save the right pixel value of this byte (even bits)
      rightpixel      = *sprite & 0b01010101;
      // Mix the right pixel of the previous byte (that now is left pixel) with 
      // the left pixel of the present byte (that now should be right pixel)
      *sprite         = (prev_rightpixel << 1) | ((*sprite & 0b10101010) >> 1);
      // Saved right pixel is stored as the previous byte right pixel, for next iteration
      prev_rightpixel = rightpixel;
      ++sprite;
   } while(--size);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Shift all pixels of a sprite to the left
//
void shiftSpritePixelsLeft(u8* sprite, u8 size) {
   u8* next_byte = sprite + 1; // Maintain a pointer to the next byte of the sprite 
   
   // Shift all bits to the left, to move sprite 1 pixel to the left
   // Assuming leftmost column is free (zeroed)
   // Iterate up to the next-to-last byte, as the last byte is a special case
   while (--size) {
      // Each byte is the mix of its right pixel (even bits) shifted to the left
      // to become left pixel, and the left pixel of the next byte shifted to the right,
      // to become right pixel.
      *sprite = ((*sprite & 0b01010101) << 1) | ((*(next_byte) & 0b10101010) >> 1);
      ++sprite; ++next_byte;
   }
   // Last byte has its right pixel shifted to the left to become left pixel, and
   // zeros added as new right pixel
   *sprite = (*sprite & 0b01010101) << 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Shift an sprite to draw it at an even or odd location
//
void shiftSprite(TEntity *e) {
   // Depending on its present status, shifting will be to the left or to the right
   // We always assume that the original sprite had its rightmost pixel column free (zeroed)
   if (e->shift == ON_EVEN_PIXEL) {     
      // Shift sprite right & update shifting status
      shiftSpritePixelsRight(e->sprite, e->w * e->h);
      e->shift = ON_ODD_PIXEL;
   } else {
      // Shift sprite left & update shifting status
      shiftSpritePixelsLeft(e->sprite, e->w * e->h);
      e->shift = ON_EVEN_PIXEL;
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Redraw an entity on the screen, synchronized with VSYNC.
//  - It erases previous location of the entity, then draws it at its new location
//
void drawEntity(TEntity *e) {
   u8 *pscr;      // Pointer to the screen byte where character will be drawn

   // First, check if we have to shift the sprite because next horizontal location
   // in pixels does not coincide with present shifting status of the Entity
   // Remaining of dividing by 2: even pixels = 0, odd pixels = 1.
   if (e->shift != (TShiftStatus) e->nx % 2)
      shiftSprite(e);

   // Wait for VSYNC
   cpct_waitVSYNC();

   // Erase the sprite drawing a 0 colour (background) box
   // Horizontal location is in pixels, so we divide it by the number of pixels per byte
   // (2 in mode 0) to give the screen location in bytes, as cpct_getScreenPtr requires
   pscr = cpct_getScreenPtr(CPCT_VMEM_START, e->x / PIXELS_PER_BYTE, e->y);
   cpct_drawSolidBox(pscr, 0x00, e->w, e->h);
   
   // Draw the sprite at its present location on the screen
   pscr = cpct_getScreenPtr(CPCT_VMEM_START, e->nx / PIXELS_PER_BYTE, e->ny);
   cpct_drawSprite(e->sprite, pscr, e->w, e->h);

   // Update sprite coordinates
   e->x = e->nx;
   e->y = e->ny;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Check User Input and perform actions
//
void checkUserInput(TEntity *e) {
   cpct_scanKeyboard_f();

   // Check input keys and produce movements 
   // Checking also that we do not exit from boundaries
   if (cpct_isKeyPressed(Key_CursorLeft) && e->nx > 0) {
      e->nx--;
   } else if (cpct_isKeyPressed(Key_CursorRight) && e->nx + e->w*PIXELS_PER_BYTE < SCR_WIDTH_PIXELS) {
      e->nx++;
   }
   if (cpct_isKeyPressed(Key_CursorUp) && e->ny > 0) {
      e->ny--;
   } else if (cpct_isKeyPressed(Key_CursorDown) && e->ny + e->h < SCR_HEIGHT_PIXELS) {
      e->ny++;
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
// APPLICATION START
//
void main(void) {
   // Create and initialize the character entity
   // Making it static is same as being global, but only visible to main.
   // As any global, we need it to be const in order to initialize it only once
   static TEntity const ec = { 
      32, 88,              // Present pixel location
      32, 88,              // Next pixel location
      SPRITE_WIDTH_BYTES,  // | Sprite Size in bytes
      SPRITE_HEIGHT_BYTES, // |
      (void*)g_character,  // Pointer to sprite definition
      ON_EVEN_PIXEL        // Pixel location is Even (32)
   };
   TEntity* e = (void*)&ec;// And then we create a non-const pointer to modify the data

   // Initialization
   cpct_disableFirmware();          // Firmware must be disabled
   cpct_setVideoMode(0);            // We will be using mode 0
   cpct_setPalette(g_palette, 5);   // We are only using 5 first palette colours
   cpct_setBorder(g_palette[0]);    // Set the border to the background colour (colour 0)
   cpct_clearScreen_f64(0x5555);    // Fastly fillup the screen with a background pattern

   // Loop forever
   while (1) {
      checkUserInput(e);  // Get user input and perform actions
      drawEntity(e);      // Draw the entity at its new location on screen
   }
}
