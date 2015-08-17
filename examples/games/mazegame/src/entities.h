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

#ifndef ENTITIES_H
#define ENTITIES_H

#include <cpctelera.h>

///////////////////////////////////////////////////////////////////////////////////
////
//// PUBLIC CONSTANTS, STRUCTURES AND DATA
////
///////////////////////////////////////////////////////////////////////////////////

// Entity statuses
typedef enum {
   ST_WALKLEFT  = 0, // Walking to the left
   ST_WALKRIGHT,     // Walking to the right
   ST_WALKUP   ,     // Walking up
   ST_WALKDOWN ,     // Walking down
   ST_HITLEFT  ,     // Hitting left
   ST_HITRIGHT ,     // Hitting right
   ST_HITUP    ,     // Hitting up
   ST_HITDOWN  ,     // Hitting down
   ST_DEAD,          // Being dead
   ST_NUMSTATUSES    // Total actions
} EEntityStatus;

// Entity information (Location, status and sprites)
typedef struct {
   u8 maze;              // Maze where the entity is located
   u8 tx, ty;            // Upper-left tile of the entity over the tilemap
   u8 nx, ny;            // Next Upper-left tile of the entity (where it will move)
   EEntityStatus status; // Status of the entity
   u8 **sprite_set;      // Set of sprites for different actions of the entity
} TEntity;

///////////////////////////////////////////////////////////////////////////////////
////
//// PUBLIC FUNCTION MEMBERS
////
///////////////////////////////////////////////////////////////////////////////////
    void ent_initialize();
    void ent_drawAll(u8* screen) __z88dk_fastcall;
    void ent_clearAll(u8* screen) __z88dk_fastcall;
TEntity* ent_getEntity(u8 id) __z88dk_fastcall;
    void ent_move(TEntity* e, i8 vx, i8 vy);
    void ent_doAction(TEntity*e, EEntityStatus action);

#endif