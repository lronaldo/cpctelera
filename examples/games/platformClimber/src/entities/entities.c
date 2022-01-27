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

#include <stdio.h>

#include <cpctelera.h>
#include "entities.h"
#include "../anim/animation.h"
#include "../anim/predefinedAnimations.h"
#include "../sprites/sprites.h"
#include "../random/random.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  DECLARATIONS OF PRIVATE FUNCTIONS FOR THIS MODULE
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

      u8 getNearestBlockID(u8 bid, u8 y);
    void applyCharacterBlockCollisions(TCharacter *c);
    void updateCharacterPhysics(TCharacter *c);
      u8 moveBlock(u8 b_idx);
    void setEntityLocation(TEntity *e, u8 x, u8 y, u8 vx, u8 vy, u8 eraseprev);
    void destroyBlock (u8 block_idx);
    void drawAnimEntity  (TEntity* e);
    void drawBlockEntity (TEntity* e);
      u8 isOverFloor(TEntity *e);
      u8 randomCreateNewBlock(u8 y, u8 h, u8 rndinc);
TEntity* newSolidBlock(u8 x, u8 y, u8 width, u8 height, u8 colour);
TCharacter* getCharacter();
TCollision* checkCollisionEntBlock(TEntity *a, TEntity *b);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  PHYSICS CONSTANTS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Gravity constants (in fixed decimal, pixels/frame)
// Assuming 1 px = 1 meter
const i16 G_gy        = 9.81 * SCALE / FPS;  // Defining gravity as 9.81 px/sec^2
const i16 G_gx        = 0;                   // No gravity on x axis, at the start
const i16 G_maxYVel   = 150 / FPS * SCALE;   // Maximum vertical velocity for an entity, 150 px/sec
const i16 G_maxXVel   =  75 / FPS * SCALE;   // Maximum horizontal velocity for an entity, 75 px/sec
const u16 G_minVel    = SCALE / 8;           // Minimum velocity for an entity (below that, velocity considered as 0)
const i16 G_jumpVel   = -150 / FPS * SCALE;  // Velocity when we start a jump
const i16 G_maxScrollVel = 25 * SCALE / FPS; // Scroll down velocity, 25 px/sec
const u8  G_airFric   = 2;                   // Friction divisor applied to horizontal movement on air
const u8  G_floorFric = 4;                   // Friction divisor applied to horizontal movement on floor
const u8  G_minX      =   4;                 // Horizontal limits of the playing area (bytes)
const u8  G_maxX      =  54;                 // 
const u8  G_minY      =   8;                 // Vertical limits of the playing area (bytes)
const u8  G_maxY      = 192;                 // 

i16 G_scrollVel;     // Velocity at which scroll goes 
u8  G_platfColour;   // Colour pattern for platforms
u16 G_score;         // Main score

// Size of the Screen and base pointer (in pixels)
//
const u8  g_SCR_WIDTH  =  80;         // Screen width in bytes (80 bytes = 160 pixels)
const u8  g_SCR_HEIGHT = 200;         // Screen height in bytes

// Define entities in the world and main character
//
#define g_MaxBlocks 16          // Maximum number of blocks at the same time
#define g_MaxMovingBlocks 2     // Maximum number of blocks moving at the same time
TEntity g_blocks[g_MaxBlocks];  // Vector with the values of the blocks
     u8 g_lastBlock;            // Last block value (next available)
     u8 g_colMinBlock;          // Minimum block in the collision range
     u8 g_colMaxBlock;          // Maximum block in the collision range
     u8 g_movingBlocks;         // Blocks that are moving

// Main Character
const TCharacter g_Character = {
   // Entity values
   { 
     { .anim = { g_walkRight, 0, 2, as_pause } }, // Initial animation
      (u8*)0xC000, (u8*)0xC000,
      0, 0, 0, 0, 0, 0, 
     { 0, 0, 0, 0, 0 },
     0, 0, as_null
   },
   es_walk,    // Walking 
   s_right     // To the right
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize entities
//   Sets up entities at their initial values
//
void initializeEntities() {
   G_platfColour = cpct_px2byteM0(8, 8);
   G_scrollVel = 3 * SCALE / FPS;  // Scroll down velocity, 3 px/sec
   g_movingBlocks = 0;

   // Initialize blocks
   g_lastBlock = 0;                                // Block ID 
   newSolidBlock( 4, 120, 50, 5, G_platfColour);   // 0 /
   newSolidBlock(14, 100, 10, 3, G_platfColour);   // 1 /
   newSolidBlock(34, 100, 10, 3, G_platfColour);   // 2 /
   newSolidBlock(26,  80,  6, 3, G_platfColour);   // 3 /
   newSolidBlock( 8,  60, 10, 3, G_platfColour);   // 4 /
   newSolidBlock(36,  55, 10, 3, G_platfColour);   // 5 /
   newSolidBlock(20,  30, 20, 3, G_platfColour);   // 6 /
   newSolidBlock( 9,  10, 10, 3, G_platfColour);   // 7 /
   newSolidBlock(44,   9,  4, 3, G_platfColour);   // 8 /
   G_score = 9; // 9 points for the 9 starting blocks

   G_platfColour = 8;

   // Initialize main character
   setEntityLocation(&g_Character.entity, 28, 120-20, 0, 0, 1);

   // Define initial collition range
   g_colMinBlock = 0;
   g_colMaxBlock = 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Getter functions
//
        u16 getScore() { return G_score; }
TCharacter* getCharacter() { return &g_Character; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Make changes in a Character for next frame, depending on 
// given actions to perform
//
void performAction(TCharacter *c, TCharacterStatus move, TCharacterSide side) {
   TEntity *e = &c->entity;   // Get entity associated to the character
   TPhysics *p = &e->phys;    // Get Physics information associated to the entity

   // Perform actions depending on the requested move
   switch(move) {
      //-------- 
      // Requested action: move left or right
      //-------- 
      case es_walk:

         // Check present action being performed, to act consequently
         switch(c->status) {
            // Things to do when we were already walking
            case es_walk:
               // If we're changing side, set up the new animation
               if ( side != c->side ) {
                  e->nAnim   = g_anim[es_walk][side]; // Next animation changes
                  c->side    = side;
               }
               e->nStatus = as_cycle;  // Make character cycle animation
               
               // << No BREAK here: we continue to es_jump, as there we perform
               // modifications to the velocity of the character

            // Things to do when we were jumping and now try to walk left or right
            case es_jump:
               // When jumping, we can move the character left or right
               // and that makes it accelerate to left (-) or right (+)
               if ( side == s_left )
                  p->vx -= SCALE;
               else
                  p->vx += SCALE;
               break;

            // Nothing to do on other cases
            default:
            ;
         }
         break;
      
      //-------- 
      // Requested Action: jump left or right
      //--------
      case es_jump:
         // We only can jump when walking on the floor
         if (c->status == es_walk) {
            e->nAnim   = g_anim[es_jump][side]; // Next animation changes
            e->nStatus = as_play;   // Jump animation only plays once
            c->side    = side;      // New side
            c->status  = es_jump;   // New status
            p->floor   = 0;         // When jumping, we left the floor
            p->vy     += G_jumpVel; // Add jump velocity to the character
         }
         break;

      //-------- 
      // Requested Action: move the floor
      //--------
      case es_moveFloor:
         // If entity is over a floor, set velocity to the floor
         if (p->floor && !p->floor->phys.vx && g_movingBlocks < g_MaxMovingBlocks) {
            ++g_movingBlocks;
            if (side == s_left)
               p->floor->phys.vx = -SCALE / 2;
            else
               p->floor->phys.vx = SCALE / 2;
         }
         break;
      
      // No move selected to perform. Check if we have to stop ongoing moves
      default:
         // When walking, stop animation
         if ( c->status == es_walk )
            e->nStatus = as_pause;     // Pause animation on next timestep
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Crop velocity according to limits (minimum and maximum)
//
void cropVelocity(i16 *v, i16 maxvel, i16 minvel) {
   // Crop depending on v being positive or negative. This is best done this
   // way as SDCC has some problems with signed / unsigned numbers
   if ( *v >= 0 ) {
      // Positive. Check limits.
      if      ( *v > maxvel ) *v = maxvel; // Crop to max. positive velocity
      else if ( *v < minvel ) *v = 0;      // Round to min positive velocity
   } else {
      // Negative
      if      ( *v < -maxvel ) *v = -maxvel;  // Crop to max negative velocity
      else if ( *v > -minvel ) *v = 0;        // Round to mix negative velocity
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Moves a block
// Parameters:
//    b_idx: Index of the block
// Returns:
//    1 if the moved block has been destroyed
//    0 if not
//
u8 moveBlock(u8 b_idx) {
   TEntity *e = &g_blocks[b_idx]; // Get next block entity
//   u8 newY;                       // New calculated Y coordinate after movement

   // Update block location acording to its Y physics
   e->phys.y += G_scrollVel;      // All blocks use this same velocity for Y axis
   e->y       = e->ny;
   e->ny      = e->phys.y / SCALE;
   
   // Check if we have to move the block graphically
   if (e->ny != e->y) {    
      // Check if the block has disappeared from the screen, to destroy it
      // Beware! Destroying a block moves all the rest in the array!
      if (e->ny > G_maxY) {
         destroyBlock(b_idx);
         return 1;         // Return informing that the block has been destroyed!
      }

      e->draw = 1;
   }

   // Update block location according to its X physics
   if (e->phys.vx) {
      e->x       = e->nx;
      e->phys.x += e->phys.vx;
      e->nx      = e->phys.x / SCALE;

      // Is there any graphical movement?
      if (e->x != e->nx) {
         // Maintain into limits
         if (e->nx < G_minX) {
            // Crossed left boundary, bounce right
            e->nx      = G_minX + 1; 
            e->phys.x  = e->nx * SCALE; 
            e->phys.vx = -e->phys.vx;
         } else if ( e->nx + e->graph.block.w >= G_maxX ) {
            // Crossed right boundary, bounce left
            e->nx      = G_maxX - e->graph.block.w;
            e->phys.x  = e->nx * SCALE;  
            e->phys.vx = -e->phys.vx;
         }
         e->draw = 1; // Set for redraw
      }
   }

   // Calculate new screen position, on draw
   if (e->draw) {
      e->pscreen  = e->npscreen;
      e->npscreen = cpct_getScreenPtr(CPCT_VMEM_START, e->nx, e->ny);
   }

   // Return signalling that the block has NOT been destroyed
   return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// scrolls the world (all the blocks) at the given velocity
//
void scrollWorld() {
   TEntity *ce = &(getCharacter()->entity);
   TPhysics *p = &ce->phys;
   u8 i;

   // Scroll all the given block entities
   for(i=0; i < g_lastBlock; ++i) {
      // Move the block and check if the block has been destroyed
      if ( moveBlock(i) ) {
         // The block has been destroyed: 
         //  1. Eliminate floor from the main character to force recalculation
         //  2. As blocks have been moved after destroy, repeat this last 
         //     block by discounting 1 from the index and continuing.
         p->floor = 0;  
         i--;
      }
   }

   // If the floor of the Character has moved, it would have been set to be drawn.
   // In that case, the character has to be moved along with its floor
   if (p->floor && p->floor->draw) {
      // Get height of the current sprite
      TAnimation *anim = &ce->graph.anim;
      u8 height = anim->frames[anim->frame_id]->height;

      // Set new physics y coordinate and vy ~= 0 (less than minVel) 
      ce->phys.y  = (p->floor->ny - height) * SCALE;
      ce->phys.vy = G_minVel - 1;
      ce->draw    = 1;
   }

   // Expand a new block, when required, at Y coordinate G_minY-3, 
   // 3 pixels high and with ce->nx random increment
   if ( g_blocks[0].draw && randomCreateNewBlock(G_minY-3, 3, ce->nx) ) {
      // If the block was created, increment score and check if we arrive
      // to a new zone (every 16 blocks, new zone)
      if ( !(++G_score & 0x0F) ) {
         // Change platform colour
         if ( ++G_platfColour > 15) 
            G_platfColour = 1;

         // Upgrade velocity
         if (G_scrollVel < G_maxScrollVel)
            ++G_scrollVel;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update the Physics of the Character: Apply gravity and movements
// Parameters:
//    c: Character for updating its physics
//
void updateCharacterPhysics(TCharacter *c) {
   TEntity  *e = &c->entity;
   TPhysics *p = &e->phys;
   TAnimation *anim = &e->graph.anim;

   // Apply gravity only if the entity is not over a floor
   if ( !isOverFloor(e) ) {
      // If the entity was over a floor, disconnect the floor from the entity
      if ( p->floor ) {
         p->floor     = 0; 
         c->status    = es_jump;
         anim->status = as_pause;
      }

      // Apply vertical gravity
      p->vy += G_gy;
   } else {
      // We are over a floor, apply innertia if floor is moving
      p->x  += p->floor->phys.vx;   // Add floor inertia
      p->vx += 1;                   // p->vx must be != 0 to enable horizontal velocity processing
   }

   // Apply horizontal gravity
   p->vx += G_gx;

   // Check if there is any movement on Y axis
   if ( p->vy ) {
      // Crop velocity to min / max limits
      cropVelocity(&p->vy, G_maxYVel, G_minVel);   

      p->y  += p->vy;         // Then add it to position
      e->ny  = p->y / SCALE;  // Calculate new screen position
   }
   
   // Check if there is any movement on x axis
   if ( p->vx ) {
      // Crop velocity to min / max limits
      cropVelocity(&p->vx, G_maxXVel, G_minVel);

      p->x += p->vx;          // Then add it to position
      e->nx = p->x / SCALE;   // And calculate new screen position

      // After movement, apply friction to velocity for next frame
      if ( c->status == es_walk )
         p->vx /= G_floorFric;   // Friction on the floor
      else
         p->vx /= G_airFric;     // Friction on air
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Gets the ID of the block entity whose Y coord is nearest to a given Y coordinate
// Parameters:
//    bid: Block id where we start comparing and searching
//      y: goal y (wanting block nearest to y)
//
u8 getNearestBlockID(u8 bid, u8 y) {
   // Check if block id (bid) is upper or lower than desired y
   if (g_blocks[bid].ny <= y) {
      // Currend block is upper, select progressively lower blocks
      // while they are upper or equal than given y
      while (bid > 0 && g_blocks[bid-1].ny <= y) --bid;
   } else {
      // Current block is lower, select progressively upper blocks
      // while they are lower than given y
      while (bid < g_lastBlock - 1 && g_blocks[bid+1].ny > y) ++bid;
   }

   // Return selected block id
   return bid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Checks collisions between main Character and Blocks and applies required changes
// Parameters:
//    c: Character for updating its physics
//
#define PXMARGIN  4
void applyCharacterBlockCollisions(TCharacter *c) {
   TEntity  *e  = &c->entity;  // Entity associated to the character
   TPhysics *p  = &e->phys;    // Physics component of the character
   u8 i;                       // Index for blocks (Also location used as limits for collision checking) 

   // Adjust entity lower limit for collision checking (nearest to the bottom of the entity)
   i = PXMARGIN + e->ny + e->graph.anim.frames[e->graph.anim.frame_id]->height; 
   g_colMinBlock = getNearestBlockID(g_colMinBlock, i);

   // Adjust entity upper limit for collision checking (nearest to the top of the entity)
   i = e->ny - PXMARGIN;
   g_colMaxBlock = getNearestBlockID(g_colMaxBlock, i);

   // Check collisions only with the blocks that are in the 
   // collision range [g_colMinBlock, g_colMaxBlock]
   for(i=g_colMinBlock; i <= g_colMaxBlock; ++i) {
      // Get block entity and calculate collision
      TEntity    *ebl = &g_blocks[i];
      TCollision *col = checkCollisionEntBlock(e, ebl);
      
      // Check if there is a collision
      if (col->w && col->h) {
         // 4 Possible collisions (up, down, left or right)

         // Lateral Collision
         if(e->x <= col->x - e->pw  || e->x >= ebl->x + ebl->pw ) {
            if (col->x > ebl->nx) 
               e->nx += col->w;    // move col->w bytes right (colliding right)
            else
               e->nx -= col->w;    // move col->w bytes left  (colliding left)

            // Update physics horizontal coordinates
            p->x  = e->nx * SCALE;
            p->vx = 0;

         // Upside Collision
         } else if (e->y < col->y - e->ph / 2) { 
            p->floor   = ebl; // Make this entity the floor
            e->nAnim   = g_anim[es_walk][c->side]; // Next animation changes
            e->nStatus = as_pause;     // Make character cycle animation
            c->status  = es_walk;
            e->ny      = col->y - e->nAnim[0]->height; // Move col->h bytes upside and 
            // Beware of jumping below 0: e->ny is unsigned!
            if (e->ny > G_maxY)  
               e->ny=G_minY;
            p->y       = e->ny * SCALE;
            p->vy      = 0;

         // Downside Collision
         } else {
            e->ny  = col->y + col->h;  // Move col->h bytes downside (ceil)
            p->y   = e->ny * SCALE;
            p->vy  = 0;
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update an entity (do animation, and change screen location)
// Parameters:
//    c: Character to be updated
//
u8 updateCharacter(TCharacter *c) {
   TEntity  *e = &c->entity;
   TPhysics *p = &e->phys;
   TAnimation *anim = &e->graph.anim;
   TAnimFrame   *af = anim->frames[anim->frame_id];
   u8         alive = 1;

   // Previous to calculations, next position is similar to current
   e->x       = e->nx;
   e->y       = e->ny;
   e->pscreen = e->npscreen;
   e->pw      = af->width;
   e->ph      = af->height;

   // Update animation. If changes sprite, then we should redraw
   if ( updateAnimation(&e->graph.anim, e->nAnim, e->nStatus) ) { 
      e->draw = 1;                        // Redraw 
      af = anim->frames[anim->frame_id];  // Get values of the new frame
      e->nAnim   = 0;                     // No next animation/animstatus
      e->nStatus = as_null;
   }

   // Update physics and check collisions
   updateCharacterPhysics(c);
   applyCharacterBlockCollisions(c);

   // Maintain into limits
   if ( e->nx <= G_minX) { 
      e->nx = G_minX + 1; 
      p->x = e->nx * SCALE; 
   } 
   else if ( e->nx + af->width >= G_maxX ) {
      e->nx = G_maxX - af->width;
      p->x  = e->nx * SCALE;  
   }
   if ( e->ny + af->height >= G_maxY ) { 
      e->ny = G_maxY - af->height;
      p->y = e->ny * SCALE;
      alive = 0;
   }
   else if ( e->ny <= G_minY ) { 
      e->ny = G_minY + 1;
      p->y = e->ny * SCALE;
   }
   

   // Check if character has moved to calculate new location and set for drawing
   if ( e->ny != e->y ) { 
      e->npscreen  = cpct_getScreenPtr(CPCT_VMEM_START, e->nx, e->ny);
      e->draw = 1;
   } else if ( e->nx != e->x ) {
      e->npscreen = e->npscreen + e->nx - e->x;
      e->draw = 1; 
   } 

   return alive;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Checks collisions between an animated entity and a block
//
TCollision* checkCollisionEntBlock(TEntity *a, TEntity *b) {
   static TCollision c;
   TAnimFrame *ani = a->graph.anim.frames [a->graph.anim.frame_id];
   TBlock     *blk = &b->graph.block;

   // No collision at the start of the check
   c.h = 0;

   // Calculate vertical collision
   {
      u8 a_bbound = a->ny + ani->height - 1;// -- bottom boundary of a
      u8 b_bbound = b->ny + blk->h - 1;     // -- bottom boundary of b

      // Calculate vertical collision
      if ( a->ny <= b->ny ) {               // Case 1: a is up, b is down
         if ( b->ny <= a_bbound ) {         // Check if b is inside the height of a
            c.y = b->ny;                    // Yes, calculate vertical collision area
            if ( b_bbound < a_bbound )
               c.h = b_bbound - c.y + 1;
            else
               c.h = a_bbound - c.y + 1;
         }
      } else {                              // Case 2: b is up, a is down
         if ( a->ny <= b_bbound ) {         // Check if a is inside the height of b
            c.y = a->ny;                    // Yes, calculate vertical collision area
            if ( b_bbound < a_bbound )
               c.h = b_bbound - c.y + 1;
            else
               c.h = a_bbound - c.y + 1;
         }
      }
   }


   // Calculate horizontal collision, only if there was vertical collision
   if (c.h) {
      u8 a_rbound = a->nx + ani->width - 1; // -- right boundary limit of a
      u8 b_rbound = b->nx + blk->w - 1;     // -- right boundary limit of b
      c.w = 0;                          // Erase previous values and set to 0

      if ( a->nx <= b->nx ) {           // Case 1: a is left, b is right
         if ( b->nx <= a_rbound ) {     // Check if b is inside the width of a
            c.x = b->nx;                // Yes, calculate horizontal collision area
            if ( b_rbound < a_rbound )
               c.w = b_rbound - c.x + 1;
            else
               c.w = a_rbound - c.x + 1;
         }
      } else {                          // Case 2: b is left, a is right
         if ( a->nx <= b_rbound ) {     // Check if a is inside the width of b
            c.x = a->nx;                // Yes, calculate horizontal collision area
            if ( b_rbound < a_rbound )
               c.w = b_rbound - c.x + 1;
            else
               c.w = a_rbound - c.x + 1;
         }     
      }
   }

   return &c;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if a entity is still over its defined floor
//
u8 isOverFloor(TEntity *e) {
   u8 over = 0;                  // Value to sign if we are over a floor or not
   TEntity *f = e->phys.floor;   // Get the pointer to the floor assigned to this entity

   // Check if there is a floor assigned to the entity (not null pointer)
   if ( f ) {
      TAnimFrame *e_a = e->graph.anim.frames[e->graph.anim.frame_id];
      // Check X boundaries
      if ( e->x <= (f->x + f->graph.block.w) &&    // X lower  than right border of the block
          (e->x + e_a->width) >= f->x           )  // X + width higher than left border of the block
         over = 1;

      // Note: We do not check Y boundaries, because they do not change until the user
      // jumps, but then the floor is disconnected automatically. 
   }

   // Inform if we are over a floor or not
   return over;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets up a new location for an entity in screen (and its velocity)
//    Parameters give information in screen coordinates and values
//
void setEntityLocation(TEntity *e, u8 x, u8 y, u8 vx, u8 vy, u8 eraseprev) {
   // Locate entity on screen
   e->npscreen   = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
   e->nx = x;
   e->ny = y;

   // Locate entity on the physics world (with integer arithmetic)
   e->phys.x    = x  * SCALE;
   e->phys.y    = y  * SCALE;
   e->phys.vx   = vx * SCALE;
   e->phys.vy   = vy * SCALE;
   
   // Make previous values of the entity equal to next
   if (eraseprev) {
      e->pscreen  = e->npscreen;
      e->x = x;
      e->y = y;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Draw a given animated entity on the screen
//
void drawAnimEntity (TEntity* e) {
   // Check if needs to be redrawn
   if ( e->draw ) {
      // Get the entity values from its current animation status
      TAnimation* anim  = &e->graph.anim;
      TAnimFrame* frame = anim->frames[anim->frame_id];
   
      // Remove trails 
      cpct_drawSolidBox(e->pscreen, 0x00, e->pw, e->ph);

      // Draw the entity
      cpct_drawSprite(frame->sprite, e->npscreen, frame->width, frame->height);

      e->draw = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Draw a given animated entity on the screen
//
void drawBlockEntity (TEntity* e){
   // Check if needs to be redrawn
   if ( e->draw ) {
      u8* sp;   // Starting video mem pointer for blocks that are in the upper non-visible zone
      u8  eraseh;// height to be erased 
      u8  drawh;// Height to draw of the box (taking into account invisible zones)

      // Get the entity values from its current animation status
      TBlock* block  = &e->graph.block;
         
      // Take into account non visible zones for drawing the blocks
      sp = e->npscreen;
      if (e->ny <= G_minY) {
         drawh = block->h + e->ny - G_minY;
         sp = cpct_getScreenPtr(CPCT_VMEM_START, e->nx, G_minY);
      } else {
         if (e->ny + block->h > G_maxY) {
            drawh  = G_maxY - e->ny;
            eraseh = G_maxY - e->y;
         } else {
            drawh  = block->h;
            eraseh = drawh;
         }

         // Remove previous entity
         if (eraseh)
            cpct_drawSolidBox(e->pscreen,  0x00, block->w, eraseh);
      }
 
      // Draw the entity
      if (drawh)
         cpct_drawSolidBox(sp, block->colour, block->w, drawh);

      e->draw = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Draw all the scene 
//
void drawAll() {
   u8  i = g_lastBlock;

   // Draw Blocks (from last to first)
   while(i--) 
      drawBlockEntity(&g_blocks[i]);

   // Draw main character
   drawAnimEntity(&g_Character.entity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Creates an inserts a new solid block, returning its entity pointer
//
TEntity* newSolidBlock(u8 x, u8 y, u8 width, u8 height, u8 colour) {
   TEntity *newEnt = 0;

   // Only can create when we are below the limit
   if (g_lastBlock < g_MaxBlocks) {
      // Create the new entity by assigning given values to the one in the entity vector
      newEnt = &g_blocks[g_lastBlock];
      newEnt->graph.block.w      = width;
      newEnt->graph.block.h      = height;
      newEnt->pw                 = width;
      newEnt->ph                 = height;
      newEnt->graph.block.colour = colour;
      setEntityLocation(newEnt, x, y, 0, 0, 1);
      newEnt->draw               = 1;
      // Synchronize with previous blocks at subscale movement
      if (g_lastBlock > 0) 
         newEnt->phys.y += g_blocks[g_lastBlock-1].phys.y % SCALE;
      newEnt->phys.bounce        = 0.85 * SCALE;
      newEnt->phys.vx            = 0;

      ++g_lastBlock;   // One more entity added to the vector
   }

   return newEnt;
}

///////////////////////////////////////////////////////////////////////////
// Destroys a block, given its index, and reorders the block vector
//
void destroyBlock(u8 i) {
   i8 nEnts = g_lastBlock - i - 1; // Entities to the right of the block

   // If the block is moving, alow one more moving block
   if (g_blocks[i].phys.vx)
      --g_movingBlocks;

   // If there are entities to the right of the block, move them 1 index to the left
   if (nEnts)
      cpct_memcpy(&g_blocks[i], &g_blocks[i+1], nEnts*sizeof(TEntity));

   // 1 less block   
   --g_lastBlock;
}

///////////////////////////////////////////////////////////////////////////
// Create new block depending on some random constraints
// Parameters:
//    y = y coordinate for the new block
//    h = height of the new block
//

// Some Useful macros (help clarify code)
#define MINPIXELSPACE   G_minY + 10
#define RAND_0_15(R) (getRandomUniform((R)) & 0x0F)
#define RAND_0_63(R) (getRandomUniform((R)) & 0x3F)

u8 randomCreateNewBlock(u8 y, u8 h, u8 rndinc) {
   u8 last_y = g_blocks[g_lastBlock-1].ny;   // y coordinate of the upmost block
   u8 created = 0;                           // Flag to signal if a new block was created

   // If there is enough pixel space, create a random number (0-31) and,
   // if it is less than last_y (18...31), create a new platform
   if ( (RAND_0_15(1) + MINPIXELSPACE) < last_y ) {
      u8 w;                          // Random Width for the new block
      u8 x = G_minX + RAND_0_63(1);  // Random X for the new block

      // If random x is out of range, modularize it to set it on range
      if (x >= G_maxX - 1) x = x - G_maxX + G_minX;

      // Create random width for the new platform (4..19)
      w = RAND_0_15(rndinc) + 4;
      
      // If rightmost pixel of the new block exceedes range, 
      //   adjust it to the limits
      if (x + w > G_maxX) w = G_maxX - x;
      
      // Create the new solid block (if there is enough space)
      if ( newSolidBlock(x, y, w, h, cpct_px2byteM0(G_platfColour, G_platfColour)) )
         created = 1;
   }

   return created;
}

// Macros are not needed anymore
#undef MINPIXELSPACE 
#undef RAND1_0_15
#undef RAND1_0_63  
