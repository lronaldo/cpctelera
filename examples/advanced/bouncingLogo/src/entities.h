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
   float vx, vy;           // Velocity
   float acum_x, acum_y;   // Acumulated movement
   float max_x, max_y;     // Max velocity
} TVelocity;

//
// Describes a visible entity on the screen. This visible entity has a sprite, that
// is to be drawn at a given videopos, along with its x, y coordinates and its 
// height and width. It also has physics, with velocity.
//
typedef struct {
   unsigned char* sprite;
   unsigned char* videopos;
   unsigned char x, y, width, height;
   TVelocity vel;
} TEntity;

//
// Define global gravity variable to make it publicly accessible
//
extern float g_gravity;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

unsigned char *getScreenPointer(unsigned char y);
char moveEntityX         (TEntity* ent, char mx, unsigned char sx);
char moveEntityY         (TEntity* ent, char my, unsigned char sy);
void entityPhysicsUpdate (TVelocity *vel, float ax, float ay);
void updateEntities      (TEntity *logo, float ax, float ay);
