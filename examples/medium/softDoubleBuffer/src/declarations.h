//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2017 Bouche Arnaud
//  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#ifndef _DECLARATIONS_H_
#define _DECLARATIONS_H_

/////////////////////////////////////////////////////////////////////////////////
// INCLUDES
#include <cpctelera.h>

// Sprites
#include <sprites/back.h>
#include <sprites/ship.h>
#include <sprites/title.h>
#include <sprites/fire.h>

// Other modules
#include <manageVideoMem.h>
#include <textDrawing.h>
#include <drawing.h>

/////////////////////////////////////////////////////////////////////////////////
// USEFUL MACROS AND CONSTANTS
//
#define SCREEN_BUFF           0x8000                // Double buffer location
#define MASK_TABLE_SIZE       0x100
#define MASK_TABLE_LOCATION   (SCREEN_BUFF - MASK_TABLE_SIZE) 
#define NEW_STACK_LOCATION    (MASK_TABLE_LOCATION - MASK_TABLE_SIZE)

// Locate ViewPort and Objects on Screen
#define SCREEN_W_BYTES        80
#define SCREEN_H              200
#define VIEW_W_PIXELS         200
#define VIEW_H_PIXELS         60
#define MODE1_PIXELS_PER_BYTE 4
#define VIEW_W_BYTES          (VIEW_W_PIXELS / MODE1_PIXELS_PER_BYTE)
#define VIEW_H_BYTES          VIEW_H_PIXELS
#define VIEW_X                ((SCREEN_W_BYTES - VIEW_W_BYTES) / 2)
#define VIEW_Y                0
#define POS_TEXT              (VIEW_Y + VIEW_H_BYTES + 20)
#define POS_INFO              (VIEW_Y + VIEW_H_BYTES + 70)
#define POS_TITLE_X           ((VIEW_W_BYTES - G_TITLE_W) / 2)
#define POS_SHIP_X            ((VIEW_W_BYTES - (G_SHIP_W / 2)) / 2)
#define POS_SHIP_Y            ((VIEW_H_BYTES - G_SHIP_H) / 2 + 5)

// Memory location definition
#define VIDEO_MEM             0
#define BUFFER_MEM            1
#define NB_BUFFERS            2

// Declare Mask Table for Mode 1
cpctm_declareMaskTable(gMaskTable);

#endif