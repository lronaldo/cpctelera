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

#include <cpctelera.h>
#include "entities.h"
#include "sprites.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  ANIMATIONS AND VALUES FOR MANAGING ENTITIES
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// All the animation frames to be used in this example
//
const TAnimFrame g_allAnimFrames[12] = {
   // Walk Right Frames
   { gc_PerseaWalk2,  8, 24, 2, 0, 4 }, // 0// Persea Walk Right 1,   change sprite, 2 cycles time
   { gc_PerseaWalk13, 8, 24, 2, 0, 4 }, // 1// Persea Walk Right 2/4, moving 1 step forward, 2 cycles time
   { gc_PerseaWalk4,  8, 24, 2, 0, 4 }, // 2// Persea Walk Right 3,   change sprite, 2 cycles time
   // Walk Left Frames
   { gc_PerseaWalk4,  8, 24,-2, 0, 4 }, // 3// Persea Walk Left 1, 1 step backwards, 2 cycles time
   { gc_PerseaWalk13, 8, 24,-2, 0, 4 }, // 4// Persea Walk left 2/4, change sprite
   { gc_PerseaWalk2,  8, 24,-2, 0, 4 }, // 5// Persea Walk Left 3, 1 step backwards, 2 cycles time
   // Fisting and Kicking animations
   { gc_PerseaFist,   9, 24, 0, 0,15 }, // 6// Persea Fist,   change sprite, 4 cycles time
   { gc_PerseaKick,   9, 24, 0, 0,20 }, // 7// Persea Fist,   change sprite, 5 cycles time
   // Receiving a Hit
   { gc_PerseaHit,    8, 24, 0, 0,20 }, // 8// Persea Hit,    change sprite, 3 cycles time
   // KO and winning
   { gc_PerseaKO,    12,  8,-4,16, 0 }, // 9// Persea Dies, move down, static
   { gc_PerseaWins,   8, 24, 0, 0, 0 }, //10// Persea Wins, change sprite, static
   // Stay 
   { gc_PerseaWalk13, 8, 24, 0, 0, 0 }  //11// Persea Stays, change frame, static
};


// Use a define for convenience
#define FF g_allAnimFrames

//
// All complete animations used in this example (NULL terminated, to know the end of the array)
//
TAnimFrame* const g_animStay[2]      = { &FF[4], 0 };
TAnimFrame* const g_animWalkRight[5] = { &FF[0], &FF[1], &FF[2], &FF[1], 0 };
TAnimFrame* const g_animWalkLeft[5]  = { &FF[3], &FF[4], &FF[5], &FF[4], 0 };
TAnimFrame* const g_animFist[3]      = { &FF[6], &FF[11], 0 };
TAnimFrame* const g_animKick[3]      = { &FF[7], &FF[11], 0 };
TAnimFrame* const g_animHit[3]       = { &FF[8], &FF[11], 0 };
TAnimFrame* const g_animWin[2]       = { &FF[10], 0 };
TAnimFrame* const g_animDie[2]       = { &FF[9], 0 };

#undef FF

//
// Animation data that will use our main character, Persea, with its initial values
//
const TAnimation g_perseaAnimation = { g_animStay, 0, 0, as_pause };

//
// Persea main entity definition, with its initial values
//
const TEntity g_persea = { &g_perseaAnimation, 0xC000, 0, 0, es_stop };

//
// Some global constants defining our world
//
const unsigned char g_SCR_WIDTH  =  80;  // Screen width in bytes (80 bytes = 160 píxels)
const unsigned char g_SCR_HEIGHT = 200;  // Screen height in bytes

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// Return the Persea Entity
//
TEntity* getPersea() {
   return (TEntity*)g_persea;
}

//
// This function returns the video memory address where a given pixel line y starts.
//
unsigned char *getScreenPointer(unsigned char y) {
   return (unsigned char *)0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048);
}

//
// Move an entity along the X axis in pixels. The entity 'ent' will be
// moved 'mx' pixels, taking into account that the entity cannot go beyond
// limits (0 and 'Screen width'). 
// If the entity tries to pass the limits, it is stopped at them.
// This function returns 1 when entity has been moved, 0 otherwise
//
char moveEntityX (TEntity* ent, char mx) {
   unsigned char moved = 0;// Tells us if the entity has been moved or not
   unsigned char umx;      // Holds the value of mx without sign (always positive)

   // Case 1: Moving to the left (negative ammount of pixels)
   if (mx < 0) {
      umx = -mx;   // umx = positive value of mx, that is negative

      // Move umx pixels to the left, taking care not to pass 0 limit
      if (umx <= ent->x) {
         ent->x        -= umx;
         ent->videopos -= umx;
         moved          = 1;
      } else if (ent->x) {
         // movement tryied to pass 0 limit, adjusting to 0
         ent->videopos -= ent->x;
         ent->x         = 0;
         moved          = 1;
      }
   // Case 2: Moving to the right (positive amount of pixels)
   } else if (mx) {
      TAnimation*   anim;
      unsigned char space_left;
      umx = mx;   // umx = mx, as both of them are positive

      // Calculate available space to move to the right
      anim = ent->anim; 
      space_left = g_SCR_WIDTH - anim->frames[anim->frame_id]->width - ent->x;


      // Check if we are trying to move more than the available space or not
      if (umx <= space_left) {
         ent->x        += umx;
         ent->videopos += umx;
         moved         = 1;
      } else if (space_left) {
         // Moving more than available space: adjusting to available space bounce
         ent->x        += space_left;
         ent->videopos += space_left;
         moved         = 1;
      }
   }

   // Report if entity has been moved
   return moved;
}

//
// Move an entity along the Y axis in pixels. The entity 'ent' will be
// moved 'my' pixels, taking into account that the entity cannot go beyond
// limits (0 and 'Screen Height'). 
// If the entity is stopped from passing limits, it will be stopped at them.
// This function returns 1 when entity has been moved, 0 otherwise
//
char moveEntityY (TEntity* ent, char my) {
   char moved = 0;
   unsigned char umy;

   // Case 1: Moving up (negative ammount of pixels)
   if (my < 0) {
      umy = -my;  // umy = possive value of my, that is negative.

      // Move umy pixels up, taking care not to pass 0 limit
      if (umy <= ent->y) {
         ent->y        -= umy;
         ent->videopos  = getScreenPointer(ent->y) + ent->x;
         moved          = 1;
      } else if ( ent->y ) {
         // movement tryied to pass 0 limit, adjusting to 0 
         ent->videopos  = (unsigned char*)0xC000 + ent->x;
         ent->y         = 0;
         moved          = 1;
      }
   // Case 1: Moving down (positive ammount of pixels)
   } else if (my) {
      unsigned char space_left;
      TAnimation*   anim;
      umy = my;   // Both umy and my are positive

      // Calculate available space to move to the right
      anim       = ent->anim;
      space_left = g_SCR_HEIGHT - anim->frames[anim->frame_id]->height - ent->y;

      // Check if we are trying to move more than the available space or not
      if (umy <= space_left) {
         ent->y  += umy;
         moved    = 1;
      } else if (space_left) {
         // Moving more than available space: adjusting to available space 
         ent->y  += space_left;
         moved    = 1;
      }
      if (moved) {
         // Recalculating video pos when y has been changed
         ent->videopos = getScreenPointer(ent->y) + ent->x;
      }
   }

   // Report moved
   return moved;
}

//
// Updates an animation
//   Returns 1 when a new frame is reached, and 0 otherwise
//
char updateAnimation(TAnimation* anim) {
   char newframe = 0;
   
   // Update only if animation is not paused or finished
   if (anim->status != as_pause && anim->status != as_end) {
      
      // Update time and, If time has finished for this frame, get next
      if ( ! --anim->time ) {
         TAnimFrame* frame;
         
         // Next frame
         newframe = 1;
         frame = anim->frames[ ++anim->frame_id ];

         // If frame is not null, we have a new frame, else animation may have ended or may recycle
         if (frame) {
            // New frame values
            anim->time = frame->time;
         } else if ( anim->status == as_cycle ) {
            // Recycle to first frame
            anim->frame_id = 0;
            anim->time     = anim->frames[0]->time;
         } else {
            // End animation
            anim->status = as_end;
         }
      }           
   }  
   
   // Report if a new frame has started
   return newframe;
}

//
// Update an entity (do animation, move it, etc)
//
void updateEntity(TEntity *ent) {
   TAnimation* anim = ent->anim;
   
   if ( updateAnimation(anim) ) {
      if ( anim->status != as_end ) {
         TAnimFrame* frame = anim->frames[anim->frame_id];
         
         // Move on X and Y if required
         if (frame->mx) moveEntityX(ent, frame->mx);
         if (frame->my) moveEntityY(ent, frame->my);
      } else {
         ent->status = es_stop;
      }  
   }
}

//
// Set a new animation status to the entity, if follows
//
void setAnimation(TEntity *ent, TEntityStatus newstatus) {
   TAnimation* anim = ent->anim;
   
   // Only set new status if different from previous one
   if ( newstatus != ent->status || anim->status == as_end ) {
      ent->status = newstatus;
      
      // Initialize new status
      switch (newstatus) {
         case es_dead:        { anim->frames = (TAnimFrame**)g_animDie;       anim->status=as_pause; break;  }
         case es_stop:        { anim->frames = (TAnimFrame**)g_animStay;      anim->status=as_pause; break;  }
         case es_walk_right:  { anim->frames = (TAnimFrame**)g_animWalkRight; anim->status=as_play;  break;  }
         case es_walk_left:   { anim->frames = (TAnimFrame**)g_animWalkLeft;  anim->status=as_play;  break;  }
         case es_fist:        { anim->frames = (TAnimFrame**)g_animFist;      anim->status=as_play;  break;  }
         case es_kick:        { anim->frames = (TAnimFrame**)g_animKick;      anim->status=as_play;  break;  }
         case es_hit:         { anim->frames = (TAnimFrame**)g_animHit;       anim->status=as_play;  break;  }
      }
      // Set values as if this was -1 frame (previous to initial 0 frame)
      // This will make updateAnimation jump to frame 0 on first update, executing frame 0 moves on enter.
      anim->frame_id = 0xFF;
      anim->time = 1;
   }
}

//
// Draw a given entity on the screen
//
void drawEntity  (TEntity* ent){
   // Get the entity values from its current animation status
   TAnimation* anim  = ent->anim;
   TAnimFrame* frame = anim->frames[anim->frame_id];

   // Draw the entity
   cpct_drawSprite(frame->sprite, ent->videopos, frame->width, frame->height);
}