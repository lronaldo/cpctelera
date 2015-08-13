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


// Mazes 
extern const u8 g_maze[8][40*50];

// Connections between mazes (255 = no connection)
extern const u8 g_mazeConnections[8][4];

//
// Identifiers for the 4 connection boundaries of a maze
//
typedef enum {
   CONNECT_UP    = 0,
   CONNECT_DOWN  = 1,
   CONNECT_LEFT  = 2,
   CONNECT_RIGHT = 3
} EConnection;


#endif