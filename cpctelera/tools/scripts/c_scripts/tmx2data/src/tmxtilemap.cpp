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
#include <iostream>
#include <cmath>

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap() { }

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap(char* tmxfilename) { loadMap(tmxfilename); }

CPCT_TMX_Tilemap::~CPCT_TMX_Tilemap() {
}

void
CPCT_TMX_Tilemap::loadMap(const char* tmxfilename) {
   // Load map and check it to be a correct TMX
   if ( ! m_map.load(tmxfilename)) 
      error( { "File ", tmxfilename, " is not a TMX, is corrupted or could not be loaded." } );

   // Check for required map properties
   if (m_map.getOrientation() != tmx::Orientation::Orthogonal)
      error( { "'", tmxfilename, "' has a map orientation different than Orthogonal. Only Orthogonal orientation is supported."} );

   // Save the filename
   m_filename = tmxfilename;

   // m_map.getVersion().upper << ", " << m_map.getVersion().lower << std::endl;   
}

void
CPCT_TMX_Tilemap::setBitsPerItem(uint8_t bits) {
   std::set<uint8_t> validbits { 1, 2, 4, 6, 8 };
   if (validbits.find(bits) == validbits.end()) 
      error( { "Invalid number of bits per item '", std::to_string(bits), "'. Valid values are: ", setToString(validbits, " ") } );
   m_bitsPerItem = bits;
}


void
CPCT_TMX_Tilemap::printSomeInfo() const {
   //m_map.getTileCount()
   //m_map.getTilesets()
   //m_map.getLayers()
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate output header file for basic C conversion (1 layer)
//
void
CPCT_TMX_Tilemap::output_basic_H(std::ostream& out) const {
   uint32_t tw = m_map.getTileCount().x;
   uint32_t th = m_map.getTileCount().y;
   uint32_t total_bytes = std::ceil(tw * th * m_bitsPerItem / 8.0);
   std::string theTime( std::ctime(&m_time) ); theTime.pop_back();

   out <<   "//";
   out << "\n// File "<< m_filename <<" converted to csv using cpct_tmx2data ["<< theTime <<"]";
   out << "\n//   * Width:  " << tw;
   out << "\n//   * Height: " << th;
   out << "\n//   * Bytes:  " << total_bytes;
   out << "\n//";
   out << "\n#include <types.h>";
   out << "\n";
   out << "\n// Generated CSV tilemap from " << m_filename;
   out << "\n//   "<< total_bytes << " bytes ("<< tw <<" x "<< th <<"), "<< (uint16_t)m_bitsPerItem <<" bits per item";
   out << "\n#define "<< m_cid <<"_W  " << tw;
   out << "\n#define "<< m_cid <<"_H  " << th;
   out << "\nextern const u8 "<< m_cid <<"["<< total_bytes <<"];";
   out << "\n";
}