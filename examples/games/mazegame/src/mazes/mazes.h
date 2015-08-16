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

///////////////////////////////////////////////////////////////////////////////////
////
//// PUBLIC CONSTANTS, STRUCTURES AND DATA
////
///////////////////////////////////////////////////////////////////////////////////

// Some sizes
#define MAZE_WIDTH_TILES   40
#define MAZE_HEIGHT_TILES  50
#define MAZE_SIZE_TILES    (MAZE_WIDTH_TILES * MAZE_HEIGHT_TILES)
#define NUM_MAZES           8

// Mazes 
extern const u8 g_maze[NUM_MAZES][MAZE_SIZE_TILES];

// Maze coordinates: Bitvector 
// Each maze has 2 coordinates (x,y) , x,y € [0,3], so 2 bits
// are required for each coordinate, 4 bits for each maze, 
// then 1 byte = 2 mazes. Then we need NUM_MAZES / 2 bytes.
extern const u8 g_mazeCoordinates[NUM_MAZES / 2];

///////////////////////////////////////////////////////////////////////////////////
////
//// PUBLIC FUNCTIONS
////
///////////////////////////////////////////////////////////////////////////////////
void maze_initialize(u8 maze_id);
void maze_setPresent(u8 maze_id);
void maze_draw(u8* screen);

#endif