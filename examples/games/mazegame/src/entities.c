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

#include "entities.h"
#include "sprites/sprites.h"
#include "mazes/mazes.h"

///////////////////////////////////////////////////////////////////////////////////
////
//// CONSTANTS, STRUCTURES AND DATA
////
///////////////////////////////////////////////////////////////////////////////////

// Entities
TEntity m_entities[10];
TEntity *m_player;
u8 m_numEnts;

///////////////////////////////////////////////////////////////////////////////////
////
//// FUNCTION MEMBERS
////
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Initialize entities
//
void ent_initialize() {
   m_player = (m_entities + 0);
   m_numEnts = 1;
   m_player->maze       = 0;
   m_player->tx         = 5;
   m_player->ty         = 5;
   m_player->nx         = 5;
   m_player->ny         = 5;
   m_player->status     = ST_WALKLEFT;
   m_player->sprite_set = g_player_tileset;
}

///////////////////////////////////////////////////////////////////////////////////
// Get a pointer to an entity
//
TEntity* ent_getEntity(u8 id) __z88dk_fastcall {
   return (m_entities + id);
}

///////////////////////////////////////////////////////////////////////////////////
// Draw entities over a buffer
//
void ent_drawAll(u8* screen) __z88dk_fastcall  {
   u8* sprite = m_player->sprite_set[ m_player->status ];

   screen = cpct_getScreenPtr(screen, 2*m_player->nx, 4*m_player->ny);
   //cpct_drawSpriteMaskedAlignedTable(sprite, screen, 6, 12, cpct_transparentMaskTable00M0);
   cpct_drawSprite(sprite, screen, 6, 12);
}

///////////////////////////////////////////////////////////////////////////////////
// Clears all entities
//
void ent_clearAll(u8* screen) __z88dk_fastcall {
   maze_drawBox(m_player->tx, m_player->ty, 3, 3, screen);
   m_player->tx = m_player->nx;
   m_player->ty = m_player->ny;
}

///////////////////////////////////////////////////////////////////////////////////
// Move an entity 
//
void ent_move(TEntity*e, i8 vx, i8 vy) {
   e->nx = e->tx + vx;
   e->ny = e->ty + vy;
}

///////////////////////////////////////////////////////////////////////////////////
// Move entity changing animation
//
void ent_doAction(TEntity*e, EEntityStatus action) {
   i8 vx=0, vy=0;
   e->status = action;
   switch (action) {
      case ST_WALKLEFT:  vx=-1; break;
      case ST_WALKRIGHT: vx= 1; break;
      case ST_WALKUP:    vy=-1; break;
      case ST_WALKDOWN:  vy= 1; break;
   }
   ent_move(e, vx, vy);
}
