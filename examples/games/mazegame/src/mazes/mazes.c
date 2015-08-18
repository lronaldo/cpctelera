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
#include "../entities.h"
#include <cpctelera.h>

///////////////////////////////////////////////////////////////////////////////////
////
//// ATTRIBUTES
////    CONSTANTS, STRUCTURES AND DATA
////

// ~20K
// 8 Mazes, 2000 bytes / maze = 16000 bytes
// Tiles = 36 * 8 = 288 b
// Tileset = 36*2 = 72 b
// MaskTable = 256 b 
// Characters = 4*648b = 2592 b
const u8 m_maze0[MAZE_SIZE_TILES] = {
   #include "maze0.csv" 
};
const u8 m_maze1[MAZE_SIZE_TILES] = { 
   #include "maze1.csv" 
};
const u8 m_maze2[MAZE_SIZE_TILES] = { 
   #include "maze2.csv" 
};
const u8 m_maze3[MAZE_SIZE_TILES] = { 
   #include "maze3.csv" 
};
const u8 m_maze4[MAZE_SIZE_TILES] = { 
   #include "maze4.csv" 
};
const u8 m_maze5[MAZE_SIZE_TILES] = { 
   #include "maze5.csv" 
};
const u8 m_maze6[MAZE_SIZE_TILES] = { 
   #include "maze6.csv" 
};
const u8 m_maze7[MAZE_SIZE_TILES] = { 
   #include "maze7.csv" 
};

// Create array of pointers to mazes. With this, getting a pointer to a concrete
// maze is much faster, as calculating the location of a pointer inside
// this array is just multiplying by 2 (the size of a pointer), whereas calculating 
// the location of a maze inside m_mazes array is multipying by 2000 (which takes 
// 16 bytes of code and lot of CPU Cycles)
static u8* const ms_mazes[NUM_MAZES] = { 
   m_maze0, m_maze1, m_maze2, m_maze3, 
   m_maze4, m_maze5, m_maze6, m_maze7
};


// Maze space: Bitvector 
// We have 8 different mazes, and we would need a way to 
// identify a void location so 9 possibilities. Then, we
// will use 4 bits for each location. Then we create a 
// circular world with 4x4 possible locations. So, we 
// require 4x4 location, 4 bits for each (1/2 byte), 
// making 4x4/2 bytes.
// 0-7 represent a maze id. F represents void location 
//
//  Layout: \x 0 1 2 3
//          y ---------
//          0 |0 1 2 -|
//          1 |3 4 5 -|
//          2 |6 7 - -|
//          3 |- - - -|
//            ---------
//
const u8 m_mazeSpace[4*4 / 2] =  {
//   01    23   
//--------------|
   0x01, 0x2F, // 0  
   0x34, 0x5F, // 1
   0x67, 0xFF, // 2
   0xFF, 0xFF  // 3
};

// Present maze (the one on screen)
u8* m_presentMaze;
u8  m_presentMazeLocation;
u8  m_redrawTimes;

///////////////////////////////////////////////////////////////////////////////////
////
//// FUNCTION MEMBERS
////

///////////////////////////////////////////////////////////////////////////////////
// Initializes mazes module
//
void maze_initialize(u8 init_maze_id) __z88dk_fastcall {
   static const mazedoors[NUM_MAZES] = {
      MD_DOWN|MD_RIGHT,       MD_LEFT|MD_DOWN|MD_RIGHT,       MD_LEFT|MD_DOWN,
      MD_UP|MD_DOWN|MD_RIGHT, MD_UP|MD_DOWN|MD_RIGHT|MD_LEFT, MD_UP|MD_LEFT,
      MD_UP|MD_RIGHT,         MD_UP|MD_LEFT
   };
   u8 i;

   m_presentMazeLocation = 0;
   m_presentMaze = maze_getMaze(init_maze_id);
   cpct_etm_setTileset2x4(g_tile_tileset);
   i=8;
   while(i--)
      maze_setDoors(maze_getMaze(i), mazedoors[i]);
   m_redrawTimes = 2;
}

///////////////////////////////////////////////////////////////////////////////////
// Returns a pointer to a given maze, knowing its id
//
u8* maze_getMaze(u8 maze_id) __z88dk_fastcall {
   // Returns a pointer to a concrete maze, given its id. 
   return ms_mazes[maze_id];
}

///////////////////////////////////////////////////////////////////////////////////
// Changes present maze, moving to one of the mazes that is touching its
//  boundaries. 
// Warning: Does not check if changing to a non-valid location
// Movements: +15 Left  +12 Up
//             +1 Right  +4 Down
//
u8* maze_moveTo(TMazeMovement movement) __z88dk_fastcall {
   u8 id;
   
   // Special movement cases for left and right borders
   if      (movement == MM_LEFT && (m_presentMazeLocation & 0x03) == 0 )
      movement = 3;
   else if (movement == MM_RIGHT && (m_presentMazeLocation & 0x03) == 3 )
      movement = 13;

   // Returns a pointer to a concrete maze, given its id. 
   m_presentMazeLocation = (m_presentMazeLocation + (u8)movement) & 0x0F;  // Module 16

   id = cpct_get4Bits(m_mazeSpace, m_presentMazeLocation);
   m_presentMaze = ms_mazes[ id ];
   return m_presentMaze;
}

///////////////////////////////////////////////////////////////////////////////////
// Returns the present maze
//
u8* maze_getPresent() {
   return m_presentMaze;
}

///////////////////////////////////////////////////////////////////////////////////
// Draws the present maze completely (normally used for first time draw, on entering)
//
void maze_draw(u8* screen) __z88dk_fastcall {
   cpct_etm_drawTilemap2x4(MAZE_WIDTH_TILES, MAZE_HEIGHT_TILES, screen, m_presentMaze);
}

///////////////////////////////////////////////////////////////////////////////////
// Checks if there is any solid tile inside a box (collision detection)
//
u8 maze_isAnySolidTile(u8 x, u8 y, u8 w, u8 h) {
   u8* tile = m_presentMaze + y*MAZE_WIDTH_TILES + x;

   while(h--) {
      x = w;
      while (x--) {
         if (*tile++ < SOLID_TILES)
            return 1;
      }
      tile += MAZE_WIDTH_TILES - w;
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// Checks if there is any solid tile inside a box (collision detection)
//
u8 maze_checkEntityCollision(TEntity *e, EEntityStatus dir) {
   switch (dir) {
      case ST_WALKLEFT:  return maze_isAnySolidTile(e->tx-1, e->ty,   1, 3);
      case ST_WALKRIGHT: return maze_isAnySolidTile(e->tx+3, e->ty,   1, 3);
      case ST_WALKUP:    return maze_isAnySolidTile(e->tx,   e->ty-1, 3, 1);
      case ST_WALKDOWN:  return maze_isAnySolidTile(e->tx,   e->ty+3, 3, 1);
      default:
         return 0;
   }
}

///////////////////////////////////////////////////////////////////////////////////
// Draws a maze tilebox to clear something
//
void maze_drawBox(u8 x, u8 y, u8 w, u8 h, u8* screen) {
   cpct_etm_drawTileBox2x4(x, y, w, h, MAZE_WIDTH_TILES, screen, m_presentMaze);
}

///////////////////////////////////////////////////////////////////////////////////
// Set door status for a given maze
//  Doors are set to be 4 tiles wide
//
void maze_setDoors(u8* maze, u8 doorStatus) {
   // Offset of the first tile of each door
   static const u16 offsets[4] = {
     (MAZE_HEIGHT_TILES-1)*MAZE_WIDTH_TILES + MAZE_WIDTH_TILES/2 - 2,   // Lower door
     MAZE_WIDTH_TILES/2 - 2,                                            // Upper door
     (MAZE_HEIGHT_TILES/2 - 1)*MAZE_WIDTH_TILES - 1,                    // Right door
     (MAZE_HEIGHT_TILES/2 - 2)*MAZE_WIDTH_TILES                         // Left door
   };
   const u8 open  = OPEN_DOOR_TILE;
   const u8 close = CLOSED_DOOR_TILE;
   u8*        tile, value, doorbit, increment=1;
   u16 const* offset = offsets;

   // Doorbit starts being 0b1000 and goes shifting right 
   // until it reaches 0. Each of the 4 bits refer to one
   // of the 4 doors (LEFT = 0001, RIGHT = 0010, UP = 0100, DOWN = 1000)
   for (doorbit = MD_DOWN; doorbit; doorbit >>= 1) {
      tile = maze + *offset++;
      value = (doorStatus & doorbit) ? open : close;
      *tile = value; tile += increment;
      *tile = value; tile += increment;
      *tile = value; tile += increment;
      *tile = value; 
      if (doorbit & 0b100) increment = MAZE_WIDTH_TILES;
   }
}

///////////////////////////////////////////////////////////////////////////////////
// Checks if the user has touched map limits and triggers map changing and redraws
//  Only works if player moves 1 tile at a time
//

// Right and down tiles that signal entering a new maze when they are passed
#define RIGHT_LIMIT (MAZE_WIDTH_TILES - PLAYER_WIDTH_TILES - 1)
#define DOWN_LIMIT  (MAZE_HEIGHT_TILES - PLAYER_HEIGHT_TILES - 1)

void maze_checkPlayerEntersNewMaze(TEntity* ply) __z88dk_fastcall {

   // Check if the player enters the maze to the left
   if (!ply->tx) {
      maze_moveTo(MM_LEFT);
      ply->tx = ply->nx = RIGHT_LIMIT;
      m_redrawTimes = 2;
   
   // Check if the player enters the maze to the right
   } else if (ply->tx > RIGHT_LIMIT) {
      maze_moveTo(MM_RIGHT);
      ply->tx = ply->nx = 1;
      m_redrawTimes = 2;

   // Check if the player enters the maze to the upside
   } else if (!ply->ty) {
      maze_moveTo(MM_UP);
      ply->ty = ply->ny = DOWN_LIMIT;
      m_redrawTimes = 2;

   // Check if the player enters the maze to the downside
   } else if (ply->ty > DOWN_LIMIT) {
      maze_moveTo(MM_DOWN);
      ply->ty = ply->ny = 1;
      m_redrawTimes = 2;
   }
}

///////////////////////////////////////////////////////////////////////////////////
// Redraws the maze only when required
//
void maze_redraw(u8* screen) __z88dk_fastcall {
   if (m_redrawTimes) {
      maze_draw(screen);
      m_redrawTimes--;
   }
}
