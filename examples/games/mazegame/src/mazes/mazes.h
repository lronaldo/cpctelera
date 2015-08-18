//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
#ifndef MAZES_H
#define MAZES_H

#include <types.h>
#include "../entities.h"

///////////////////////////////////////////////////////////////////////////////////
////
//// PUBLIC CONSTANTS, STRUCTURES AND DATA
////
///////////////////////////////////////////////////////////////////////////////////

// Some constants
#define MAZE_WIDTH_TILES   40
#define MAZE_HEIGHT_TILES  50
#define MAZE_SIZE_TILES    (MAZE_WIDTH_TILES * MAZE_HEIGHT_TILES)
#define NUM_MAZES           8
#define SOLID_TILES        12
#define OPEN_DOOR_TILE     29
#define CLOSED_DOOR_TILE    0

//
// Movements from maze to maze 
//   These are performed in a circular 4x4 world, so
//   they are module 16 movements.
//
typedef enum {
   MM_RIGHT =  1, // x + 1
   MM_LEFT  = 15, // (x + 15) % 16 = (x - 1) % 16
   MM_DOWN  =  4, // x + 4
   MM_UP    = 12  // (x + 12) % 16 = (x - 4) % 16
} TMazeMovement;

//
// Describes bits identifying open doors in a maze
//
typedef enum {
   MD_LEFT   = 0b0001,  
   MD_RIGHT  = 0b0010,
   MD_UP     = 0b0100,
   MD_DOWN   = 0b1000
} TMazeDoorFlags;


///////////////////////////////////////////////////////////////////////////////////
////
//// PUBLIC FUNCTIONS
////
///////////////////////////////////////////////////////////////////////////////////
void maze_initialize(u8 maze_id) __z88dk_fastcall;
 u8* maze_getMaze(u8 maze_id) __z88dk_fastcall;
 u8* maze_getPresent();
 u8* maze_moveTo(TMazeMovement movement) __z88dk_fastcall;
void maze_draw(u8* screen) __z88dk_fastcall;
  u8 maze_checkEntityCollision(TEntity *e, EEntityStatus dir);
void maze_drawBox(u8 x, u8 y, u8 w, u8 h, u8* screen);
void maze_setDoors(u8* maze, u8 doorStatus);

#endif