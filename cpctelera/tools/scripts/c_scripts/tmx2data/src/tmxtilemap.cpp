//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
   
#include "tmxtilemap.h"
#include <helpers.h>
#include <cstdint>
#include <set>

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap() { }

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap(char* tmxfilename) { loadMap(tmxfilename); }

CPCT_TMX_Tilemap::~CPCT_TMX_Tilemap() {
}

void
CPCT_TMX_Tilemap::loadMap(char* tmxfilename) {
   if ( ! m_map.load(tmxfilename)) 
      error( { "<<ERROR>>: File ", tmxfilename, " is not a TMX, is corrupted or could not be loaded." } );
}

void
CPCT_TMX_Tilemap::setBitsPerItem(uint8_t bits) {
   std::set<uint8_t> validbits { 1, 2, 4, 6, 8 };
   if (validbits.find(bits) == validbits.end()) 
      error( { "<<ERROR>>: Invalid number of bits per item '", std::to_string(bits), "'. Valid values are: ", setToString(validbits, " ") } );
   m_bitsPerItem = bits;
}

