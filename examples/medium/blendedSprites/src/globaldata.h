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

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <cpctelera.h>
#include "draw.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// MACROS & CONSTANTS
///   Values and operations used everywhere in this application.
///
/// SCR_WIDTH:
/// SCR_HEIGHT: Size of the full screen video memory in bytes
/// BG_WIDTH:
/// BG_HEIGHT:  Size of the background image in bytes
/// BG_X BG_Y:  (x,y) byte coordinates of the upper-left corner
///             of the background image
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#define SCR_WIDTH   80
#define SCR_HEIGHT 200
#define BG_HEIGHT  128
#define BG_WIDTH    80
#define BG_X         0
#define BG_Y        72

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// DEFINED TYPES
///  Structured information to be used throughout the application
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// TBLend: Associates a 3-character name to every blending mode
//
typedef struct {
   CPCT_BlendMode blendmode;  // CPCTelera Blending mode
   u8             name[4];    // Name of the blending mode (3-characters)
} TBlend;

// TItem: Defines the properties of a Item (a Sprite and its Name)
//
typedef struct {
   u8* sprite;     // Pointer to the sprite that identifies the item
   u8  name[7];    // Name of the item
} TItem;

// TKeyStatus: Defines 4 possible statuses for a given Key. These statuses 
//     let us detect the precise moment when a key is pressed or released.
//
typedef enum {
     KeySt_Free          // Not pressed
   , KeySt_Pressed       // Pressed "just now"
   , KeySt_StillPressed  // Pressed (maintained)
   , KeySt_Released      // Released "just now"
} TKeyStatus;

// TKey: Associates a key with its status and action to trigger.
//
typedef struct {
   cpct_keyID  key;       // CPCtelera Key identifier
   TKeyStatus  status;    // Status of the key
   void (*action) ();     // Associated action (Pointer to void function)
} TKey;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
///  Declarations of global data elements that will be accessed
/// by other modules of the application
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Array containing all possible blending modes
//
#define G_NBLENDMODES   9
extern const TBlend g_blendModes[G_NBLENDMODES];

// Array containing all items that can be displayed
//
#define G_NITEMS  4
extern const TItem g_items[G_NITEMS];

// Array containing all keys that conform user input, along
//    with their associated actions to perform when pressed
//
#define G_NKEYS   4
extern const TKey g_keys[G_NKEYS];

// Palette array, with all colours used by this application
//
#define G_NCOLOURS   11
extern const u8 g_palette[G_NCOLOURS];

// Global variables used to know current selection (Item and Blend mode)
//
extern u8 g_selectedItem;
extern u8 g_selectedBlendMode;

#endif
