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

#include <cpctelera.h>
#include "entities.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// Move an entity along the X axis in pixels. The entity 'ent' will be
// moved 'mx' pixels, taking into account that the entity cannot go beyond
// limits (0 and 'sx'). 
// If the entity is stopped from passing limits, it is considered to have
// "bounced" off the limits and that is reported returning a 1 (bounced=1).
// Otherwise, the function returns 0 (bounced=0).
//
i8 moveEntityX (TEntity* ent, i8 mx, u8 sx) {
   i8 bounced = 0;

   // Case 1: Moving to the left (negative ammount of pixels)
   if (mx < 0) {
      // Convert mx to unsigned_mx, as SDCC has problems adding signed values to unsigned ones
      u8 umx = -mx;

      // Move umx pixels to the left, taking care not to pass 0 limit
      if (umx <= ent->x) {
         ent->x        -= umx;
         ent->videopos -= umx;
      } else {
         // movement tryied to pass 0 limit, adjusting to 0 and reporting bounce
         ent->videopos -= ent->x;
         ent->x         = 0;
         bounced = 1;
      }
   // Case 2: Moving to the right (positive amount of pixels)
   } else if (mx) {
      // Calculate available space to move to the right
      u8 space_left = sx - ent->width - ent->x;
      u8 umx = mx;

      // Check if we are trying to move more than the available space or not
      if (umx > space_left) {
         // Moving more than available space: adjusting to available space and reporting bounce
         ent->x        += space_left;
         ent->videopos += space_left;
         bounced = 1;
      } else {
         ent->x        += umx;
         ent->videopos += umx;
      }
   }

   // Report if bounce has happened or not
   return bounced;
}


//
// Move an entity along the Y axis in pixels. The entity 'ent' will be
// moved 'my' pixels, taking into account that the entity cannot go beyond
// limits (0 and 'sy'). 
// If the entity is stopped from passing limits, it is considered to have
// "bounced" off the limits and that is reported returning a 1 (bounced=1).
// Otherwise, the function returns 0 (bounced=0).
//
i8 moveEntityY (TEntity* ent, i8 my, u8 sy) {
   i8 bounced = 0;

   // Case 1: Moving up (negative ammount of pixels)
   if (my < 0) {
      // Convert my to unsigned_my (umy), as SDCC has problems adding signed values to unsigned ones
      u8 umy = -my;

      // Move umy pixels up, taking care not to pass 0 limit
      if (umy <= ent->y) {
         ent->y        -= umy;
         ent->videopos  = cpct_getScreenPtr(CPCT_VMEM_START, ent->x, 2*ent->y);
      } else {
         // movement tryied to pass 0 limit, adjusting to 0 and reporting bounce
         ent->videopos  = CPCT_VMEM_START + ent->x;
         ent->y         = 0;
         bounced = 1;
      }
   // Case 1: Moving down (positive ammount of pixels)
   } else if (my) {
      // Calculate available space to move to the right
      u8 space_left = sy - (ent->height>>1) - ent->y;
      u8 umy = my;

      // Check if we are trying to move more than the available space or not
      if (umy > space_left) {
         // Moving more than available space: adjusting to available space and reporting bounce
         ent->y  = sy - (ent->height>>1);
         bounced = 1;
      } else {
         ent->y += umy;
      }
      // Recalculating video pos when y has been changed
      ent->videopos = cpct_getScreenPtr(CPCT_VMEM_START, ent->x, 2*ent->y);
   }

   return bounced;
}

//
// Updates physical values of an entity: calculates accumulated movement
// using current velocity and updates current velocity taking into account
// given acceleration and gravity. Takes care of velocity limits not being
// exceeded.
//
f32 g_gravity;
void entityPhysicsUpdate (TVelocity *vel, f32 ax, f32 ay) {
   // Update velocity using given acceleration
   vel->vx += ax;
   vel->vy += ay;

   // Add up the force of gravity
   vel->vy += g_gravity;

   // Check if velocity limits have been exceeded and crop
   if      (vel->vx >  vel->max_x) vel->vx= vel->max_x;
   else if (vel->vx < -vel->max_x) vel->vx=-vel->max_x;
   if      (vel->vy >  vel->max_y) vel->vy= vel->max_y;
   else if (vel->vy < -vel->max_y) vel->vy=-vel->max_y;

   // Increase acumulated movement as an effect of current velocity
   vel->acum_x += vel->vx;
   vel->acum_y += vel->vy;
}


//
// Update entities based on physics status (velocity and given acceleration)
//
void updateEntities(TEntity *logo, f32 ax, f32 ay) {
   const f32 bounceCoefficient=0.85;
   i8 dx=0, dy=0;

   // Update logo physics (accumulated movement and velocity)
   entityPhysicsUpdate(&logo->vel, ax, ay);

   // Check if logo accumulated movement is more than 1 
   // either on X or on Y axis. If it is, take note and move 1 unit.
   if      (logo->vel.acum_x > 1 ) { dx =  1; logo->vel.acum_x-=1.0; }
   else if (logo->vel.acum_x < -1) { dx = -1; logo->vel.acum_x+=1.0; }
   if      (logo->vel.acum_y > 1 ) { dy =  1; logo->vel.acum_y-=1.0; }
   else if (logo->vel.acum_y < -1) { dy = -1; logo->vel.acum_y+=1.0; }

   // Make movements on X and Y. Screen is considered to be 80x100 
   // (instead of 160x200) because we always move 2 pixels in each 
   // direction (as each byte encodes 2 pixels, and we move in bytes)
   if (moveEntityX(logo, dx,  80) ) {
      logo->vel.vx = -logo->vel.vx*bounceCoefficient;
   }
   if (moveEntityY(logo, dy, 100) ) {
      logo->vel.vy = -logo->vel.vy*bounceCoefficient;
   }
}
