//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#include <types.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  DATA STRUCTURES
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// Data to manage a body that moves with a given float velocity.
//   - The body accumulates advance in x and y (acum_x, acum_y) until it has moved
//     more than 1 unit, what causes it to move 1 pixel on the screen.
//   - max_x and max_y control de maximum possible velocity the body may have.
//
typedef struct {
   f32 vx, vy;           // Velocity
   f32 acum_x, acum_y;   // Acumulated movement
   f32 max_x, max_y;     // Max velocity
} TVelocity;

//
// Describes a visible entity on the screen. This visible entity has a sprite, that
// is to be drawn at a given videopos, along with its x, y coordinates and its 
// height and width. It also has physics, with velocity.
//
typedef struct {
   const u8* sprite;
   u8* videopos;
   u8 x, y, width, height;
   TVelocity vel;
} TEntity;

//
// Define global gravity variable to make it publicly accessible
//
extern f32 g_gravity;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

  u8 *getScreenPointer   (u8 y);
  i8 moveEntityX         (TEntity* ent, i8 mx, u8 sx);
  i8 moveEntityY         (TEntity* ent, i8 my, u8 sy);
void entityPhysicsUpdate (TVelocity *vel, f32 ax, f32 ay);
void updateEntities      (TEntity *logo,  f32 ax, f32 ay);
