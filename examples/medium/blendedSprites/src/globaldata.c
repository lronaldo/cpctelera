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

#include <cpctelera.h>
#include "globaldata.h"
#include "items.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES & CONSTANTS
///  Definition of the initial values for all global variables
/// and/or constants
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Array containing all possible CPCtelera blending modes 
// along with a 3-character name associated to every one.
//
const TBlend g_blendModes[G_NBLENDMODES] = {
     { CPCT_BLEND_XOR, "XOR" } , { CPCT_BLEND_AND, "AND" }
   , { CPCT_BLEND_OR,  "OR " } , { CPCT_BLEND_ADD, "ADD" }
   , { CPCT_BLEND_SUB, "SUB" } , { CPCT_BLEND_LDI, "LDI" }
   , { CPCT_BLEND_ADC, "ADC" } , { CPCT_BLEND_SBC, "SBC" }
   , { CPCT_BLEND_NOP, "NOP" }
};

// Array containing all possible Items to be displayed,
// their sprites and names
//
const TItem g_items[G_NITEMS] = { 
     { g_items_0, " Skull" } , { g_items_1, " Paper" }
   , { g_items_2, "Potion" } , { g_items_3, "   Cat" }
};

// Palette containing all selected colours as hardware values.
//
const u8 g_palette[G_NCOLOURS] = { 
    HW_BLACK         , HW_BLUE        , HW_RED
   ,HW_BRIGHT_RED    , HW_GREEN       , HW_YELLOW
   ,HW_WHITE         , HW_PASTEL_BLUE , HW_PASTEL_CYAN
   ,HW_PASTEL_YELLOW , HW_BRIGHT_WHITE
};

// Forward declaration of some functions. The compiler only needs to
// know that this functions exist (they are defined in main.c) to 
// be able to link them later on. Once compiler knows they exist,
// they can be added to the g_keys array.
//
void selectNextItem();
void selectNextBlendMode();

// Array containing all user keys with an initial status of Free and
// an associated action (pointer to a function) to perform when pressed
//
const TKey g_keys[G_NKEYS] = {
     { Key_Space , KeySt_Free , drawCurrentSpriteAtRandom } 
   , { Key_Esc   , KeySt_Free , drawBackground            }
   , { Key_1     , KeySt_Free , selectNextItem            } 
   , { Key_2     , KeySt_Free , selectNextBlendMode       } 
};

// Definition of global variables used to select current Item and Blend Mode.
// No value is associated to them because it would be ignored by the compiler.
// Their initial values are set on initialization routines.
//
u8 g_selectedItem;
u8 g_selectedBlendMode;
