//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2021 Bouche Arnaud (@Arnaud6128)
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

#ifndef _DECLARATIONS_H_
#define _DECLARATIONS_H_

/////////////////////////////////////////////////////////////////////////////////
// INCLUDES
#include <cpctelera.h>

// Sprites
#include "sprites/g_palette.h"
#include "sprites/balloon.h"
#include "sprites/roof.h"
#include "sprites/cloud.h"
#include "sprites/square.h"
#include "sprites/star_trans.h"
#include "sprites/circle_trans.h"

// Other modules
#include "manageVideoMem.h"
#include "drawing.h"

/////////////////////////////////////////////////////////////////////////////////
// USEFUL MACROS AND CONSTANTS
//

// Memory location definition
enum
{
    VIDEO_MEM,
    BUFFER_MEM,
    NB_BUFFERS
};

// Boolean definition
#define BOOL                u8
#define TRUE                1
#define FALSE               0

// Double buffer location
#define SCREEN_BUFF         0x8000
#define VMEM_SIZE           0x4000

// Mask table location
#define MASK_TABLE_LOC      (SCREEN_BUFF - 0x100) // 0x100 = Size of Mask Table

// New stack location
#define NEW_STACK_LOC       (MASK_TABLE_LOC - 0x100) // 0x100 = Size of Stack

// Screen size
#define SCREEN_CY           200
#define SCREEN_CX           80

#define VIEW_TOP            0
#define VIEW_DOWN           (SCREEN_CY - 20)

#define POS_CLOUD_X         0
#define POS_CLOUD_Y         20

#define NB_BALLOONS         8
#define BALLOON_TRAIL       8

#define BALLOON_ACTIVE      1
#define BALLOON_INACTIVE    0

#define NB_STARS            10
#define NB_COLORS_STAR      7

/////////////////////////////////////////////////////////////////////////////////
// Mask Table Definition for Mode 0
//
cpctm_declareMaskTable(gMaskTable);

#endif