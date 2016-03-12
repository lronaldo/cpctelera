//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#include "img/scifi_bg.h"
#include "img/items.h"

// Macro constants
#define SCR_VMEM   (u8*)0xC000
#define SCR_WIDTH   80
#define SCR_HEIGHT 200
#define BG_HEIGHT  128
#define BG_WIDTH    80
#define BG_X         0
#define BG_Y        72

// Defined types
typedef struct {
   CPCT_BlendFunction blendmode;
                   u8 name[4];
} TBlend;

typedef struct {
   u8* sprite;
   u8  name[7];
} TItem;

typedef enum {
     KeySt_Free          // Not pressed
   , KeySt_Pressed       // Recently pressed
   , KeySt_StillPressed  // Pressed (maintained)
   , KeySt_Released      // Recently released
} TKeyStatus;

typedef struct {
   cpct_keyID  key;       // Check to check
   TKeyStatus  status;    // Status of the key
   void(*action)();       // Associated action
} TKey;

// Blending Modes
#define G_NBLENDMODES   9
const TBlend g_blendModes[G_NBLENDMODES] = {
     { CPCT_BLEND_XOR, "XOR" } , { CPCT_BLEND_AND, "AND" }
   , { CPCT_BLEND_OR,  "OR " } , { CPCT_BLEND_ADD, "ADD" }
   , { CPCT_BLEND_SUB, "SUB" } , { CPCT_BLEND_LDI, "LDI" }
   , { CPCT_BLEND_ADC, "ADC" } , { CPCT_BLEND_SBC, "SBC" }
   , { CPCT_BLEND_NOP, "NOP" }
};

// Sprites (items)
#define G_NITEMS   4
const TItem g_items[G_NITEMS] = { 
     { g_items_0, "Skull"  } , { g_items_1, "Paper" }
   , { g_items_2, "Potion" } , { g_items_3, "Cat"   }
};

// Firmware palette values
#define G_NCOLOURS  11
const u8 g_palette[G_NCOLOURS] = { 
    HW_BLACK         , HW_BLUE        , HW_RED
   ,HW_BRIGHT_RED    , HW_GREEN       , HW_YELLOW
   ,HW_WHITE         , HW_PASTEL_BLUE , HW_PASTEL_CYAN
   ,HW_PASTEL_YELLOW , HW_BRIGHT_WHITE
};

// Function forward declarations
void drawBackground();
void drawCurrentSpriteAtRandom();
void selectNextItem();
void selectNextBlendMode();

// Keyboard Status
#define G_NKEYS   4
const TKey g_keys[G_NKEYS] = {
     { Key_Space , KeySt_Free , drawCurrentSpriteAtRandom } 
   , { Key_Esc   , KeySt_Free , drawBackground            }
   , { Key_1     , KeySt_Free , selectNextItem            } 
   , { Key_2     , KeySt_Free , selectNextBlendMode       } 
};

// Global variables
u8 g_selectedItem;
u8 g_selectedBlendMode;

/////////////////////////////////////////////////////////////////////////
// drawBackground
//    Draws the background from pixel line 72 onwards
//
void drawBackground() {
   u8* p = cpct_getScreenPtr(SCR_VMEM, BG_X, BG_Y);
   cpct_drawSprite(g_scifi_bg_0, p             , BG_WIDTH/2, BG_HEIGHT);
   cpct_drawSprite(g_scifi_bg_1, p + BG_WIDTH/2, BG_WIDTH/2, BG_HEIGHT);
}

/////////////////////////////////////////////////////////////////////////
// drawSpriteMixed
//    draws a (sprite) of size (w,h) at a given location (x,y) using
// a given blending (mode)
//
void drawSpriteMixed(CPCT_BlendFunction mode, u8* sprite, u8 x, u8 y, u8 w, u8 h) {
   u8* p = cpct_getScreenPtr(SCR_VMEM, x, y);
   cpct_setDrawSpriteBlendFunction(mode);
   cpct_drawSpriteBlended(p, h, w, sprite);
}

/////////////////////////////////////////////////////////////////////////
// drawCurrentSpriteAtRandom
//    draws the current selected sprite using the current selected blending
// mode at a random (x,y) location inside the map
//
void drawCurrentSpriteAtRandom() {
   u8 x, y;

   // Random coordinates
   x = cpct_rand() % (BG_WIDTH  - 4) + BG_X;
   y = cpct_rand() % (BG_HEIGHT - 8) + BG_Y;

   // Draw the sprite
   drawSpriteMixed(  g_blendModes[g_selectedBlendMode].blendmode
                   , g_items[g_selectedItem].sprite
                   , x, y, 4, 8);
}

/////////////////////////////////////////////////////////////////////////
// Initialization routine
//    Disables firmware, initializes palette and video mode
//
void initialize (){ 
   // Disable firmware to prevent it from interfering
   cpct_disableFirmware();
   
   // 1. Set the palette colours using hardware colour values
   // 2. Set border colour to black
   // 3. Set video mode to 0 (160x200, 16 colours)
   cpct_setPalette(g_palette, G_NCOLOURS);
   cpct_setBorder (HW_BLACK);
   cpct_setVideoMode(0);

   // Set initial selections
   g_selectedItem      = 0xFF;
   g_selectedBlendMode = 0xFF;
   selectNextItem();
   selectNextBlendMode();
}

/////////////////////////////////////////////////////////////////////////
// updateKeyboardStatus
//    Checks user input and updates status of the relevant keys of 
// the keyboard
//
void updateKeyboardStatus() {
   u8 i;
   
   cpct_scanKeyboard();

   for(i=0; i < G_NKEYS; i++) {
      TKey *k    = &(g_keys[i]);

      if (cpct_isKeyPressed(k->key)) {
         switch(k->status) {
            case KeySt_Pressed:  { k->status = KeySt_StillPressed; break; }
            
            case KeySt_Free:
            case KeySt_Released: { k->status = KeySt_Pressed; }
         }
      } else {
         switch(k->status) {
            case KeySt_Pressed:
            case KeySt_StillPressed: { k->status = KeySt_Released; break; }
            
            case KeySt_Released:     { k->status = KeySt_Free; }
         }         
      }
   }
}

void selectNextItem() {
   u8 *p = cpct_getScreenPtr(SCR_VMEM, 16, 60);
   g_selectedItem = (++g_selectedItem) % G_NITEMS;
   cpct_drawSprite(g_items[g_selectedItem].sprite, p, 4, 8);
}

void selectNextBlendMode() {
   u8 *p = cpct_getScreenPtr(SCR_VMEM, 52, 60);
   g_selectedBlendMode = (++g_selectedBlendMode) % G_NBLENDMODES;
   cpct_drawStringM0(g_blendModes[g_selectedBlendMode].name, p, 8, 0);
}

/////////////////////////////////////////////////////////////////////////
// performUserActions
//    Checks user input and performs selected actions
//
void performUserActions() {
   u8 i;

   for(i = 0; i < G_NKEYS; i++) {
      if (g_keys[i].status == KeySt_Released)
         g_keys[i].action();
   }
}

void drawUserInterfaceMessages() {
   u8 *p;

   p = cpct_getScreenPtr(SCR_VMEM, 0,  0);
   cpct_drawStringM0("[Space]"   , p    , 3, 0);
   cpct_drawStringM0("Draw Item" , p+32 , 9, 0);
   p = cpct_getScreenPtr(SCR_VMEM, 0, 15);
   cpct_drawStringM0("[1] [2]"   , p    , 3, 0);
   cpct_drawStringM0("Select"    , p+32 , 9, 0);
   p = cpct_getScreenPtr(SCR_VMEM, 0, 30);
   cpct_drawStringM0("[Esc]"     , p    , 3, 0);
   cpct_drawStringM0("Clear"     , p+32 , 9, 0);

   p = cpct_getScreenPtr(SCR_VMEM, 0, 50);
   cpct_drawStringM0("   Item     Blend   ", p, 1, 6);
}

/////////////////////////////////////////////////////////////////////////
// Main entry point of the application
//
void main(void) {
   initialize();  // Initialize screen, palette and background
   drawBackground();
   drawUserInterfaceMessages();

   while(1) {
      updateKeyboardStatus();
      performUserActions();
      while(cpct_count2VSYNC() & 0xFE);
   }
}
