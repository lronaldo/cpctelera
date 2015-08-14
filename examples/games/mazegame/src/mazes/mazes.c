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

#include "mazes.h"
#include "../sprites/tiles.h"
#include <cpctelera.h>

// ~20K
// 8 Mazes, 2000 bytes / maze = 16000 bytes
// Tiles = 36 * 8 = 288 b
// Tileset = 36*2 = 72 b
// MaskTable = 256 b 
// Characters = 4*648b = 2592 b
const u8 g_maze[NUM_MAZES][MAZE_SIZE_TILES] = {
   { 
     #include "maze0.csv"
   },{
     #include "maze1.csv"
   },{
     #include "maze2.csv"
   },{
     #include "maze3.csv"
   },{
     #include "maze4.csv"
   },{
     #include "maze5.csv"
   },{
     #include "maze6.csv"
   },{
     #include "maze7.csv"
   }
};

//
// Connections between mazes (255 = no connection)
//  Layout: -------
//          |0 1 2|
//          |3 4 5|
//          |6 7  |
//          -------
//
const u8 g_mazeConnections[NUM_MAZES][NUM_CONNECTIONS] = {
   // Up  | Down | Left | Right
   //-------------------------
   { 255,     3,   255,     1   },  // Maze 0 connections
   { 255,     4,     0,     2   },  // Maze 1 connections
   { 255,     5,     1,   255   },  // Maze 2 connections
   {   0,     6,   255,     4   },  // Maze 3 connections
   {   1,     7,     3,     5   },  // Maze 4 connections
   {   2,   255,     4,   255   },  // Maze 5 connections
   {   3,   255,   255,     7   },  // Maze 6 connections
   {   4,   255,     6,   255   }   // Maze 7 connections
};


// Present maze (the one on screen)
u8* m_presentMaze;

////////////////////////////////////////////////////////////////////////////////////////////
// Initializes mazes module
//
void maze_initialize(u8 init_maze_id) {
   m_presentMaze = (u8*)g_maze[init_maze_id];
   cpct_etm_setTileset2x4(g_tileset);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Draws the present maze completely (normally used for first time draw, on entering)
//
void maze_draw(u8* screen) {
   cpct_etm_drawTilemap2x4(MAZE_WIDTH_TILES, MAZE_HEIGHT_TILES, screen, m_presentMaze);
}
