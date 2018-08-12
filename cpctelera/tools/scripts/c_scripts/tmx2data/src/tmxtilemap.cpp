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
#include <ctime>

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap() { }

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap(char* tmxfilename) { loadMap(tmxfilename); }

CPCT_TMX_Tilemap::~CPCT_TMX_Tilemap() {
}

void
CPCT_TMX_Tilemap::loadMap(const char* tmxfilename) {
   // Load map and check it to be a correct TMX
   if ( ! m_map.load(tmxfilename)) 
      error( { "File ", tmxfilename, " is not a TMX, is corrupted or could not be loaded." } );

   // Check for required Orientation
   if (m_map.getOrientation() != tmx::Orientation::Orthogonal)
      error( { "'", tmxfilename, "' has a map orientation different than Orthogonal. Only Orthogonal orientation is supported."} );

   // Check map for at least having 1 visible Layer
   m_visibleLayers = 0;
   for(const auto& l : m_map.getLayers()) { if ( l->getVisible() ) ++m_visibleLayers; } 
   if (! m_visibleLayers)
      error( { "'", tmxfilename, "' has 0 *visible* tilemap layers. At least 1 visible tilemap layer is required. (Remember: only *visible* layers are converted)"} );

   // Save the filename and time at which the file has been interpreted
   m_filename = tmxfilename;
   std::time_t now = std::time(nullptr);
   m_theTime = std::ctime( &now );
   m_theTime.pop_back();

   // Calculate the parameters of the tilemap
   m_tw = m_map.getTileCount().x;
   m_th = m_map.getTileCount().y;
   m_total_bytes = std::ceil(m_tw * m_th * m_bitsPerItem / 8.0);

   // m_map.getVersion().upper << ", " << m_map.getVersion().lower << std::endl;   
}

void
CPCT_TMX_Tilemap::setBitsPerItem(uint8_t bits) {
   std::set<uint8_t> validbits { 1, 2, 4, 6, 8 };
   if (validbits.find(bits) == validbits.end()) 
      error( { "Invalid number of bits per item '", std::to_string(bits), "'. Valid values are: ", setToString(validbits, " ") } );

   // Update bits per item and total bytes
   m_bitsPerItem = bits;
   m_total_bytes = std::ceil(m_tw * m_th * m_bitsPerItem / 8.0);
}

void
CPCT_TMX_Tilemap::printSomeInfo() const {
   //m_map.getTileCount()
   //m_map.getTilesets()
   //m_map.getLayers()
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generates an output code header for either C or H files
//
void
CPCT_TMX_Tilemap::output_C_code_header(std::ostream& out) const {
   out <<   "//";
   out << "\n// File "<< m_filename <<" converted to csv using cpct_tmx2data ["<< m_theTime <<"]";
   out << "\n//   * Visible Layers:  " << m_visibleLayers;
   out << "\n//   * Layer Width:     " << m_tw;
   out << "\n//   * Layer Height:    " << m_th;
   out << "\n//   * Bits per tile:   " << (uint16_t)m_bitsPerItem;
   out << "\n//   * Layer Bytes:     " << m_total_bytes << " ("<< m_tw <<" x "<< m_th <<" items, "<< (uint16_t)m_bitsPerItem <<" bits per item)";
   out << "\n//   * Total Bytes:     " << m_total_bytes * m_visibleLayers << " ("<< m_total_bytes <<" x "<< m_visibleLayers <<", bytes per layer times layers)";
   out << "\n//";
   out << "\n#include <types.h>";
   out << "\n";
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate output header file for basic C conversion
//
void
CPCT_TMX_Tilemap::output_basic_H(std::ostream& out) const {
   const auto& layers = m_map.getLayers();
   output_C_code_header(out);
   
   // Output declarations
   out << "\n//#### Width and height constants ####";
   out << "\n#define "<< m_cid <<"_W  " << m_tw;
   out << "\n#define "<< m_cid <<"_H  " << m_th;
   out << "\n";
   out << "\n//#### Converted layer tilemaps ####";
   out << "\n//   Visible layers: " << layers.size();
   out << "\n//";
   if (layers.size() == 1) {
      out << "\nextern const u8 "<< m_cid <<"["<< m_total_bytes <<"];";
   } else {
      out << "\n";
      out << "\n// General layer array";
      out << "\nextern const u8 "<< m_cid <<"["<< m_visibleLayers <<"]["<< m_total_bytes <<"];";
      // Output constants for all layers
      uint16_t nvisl = 0;
      out << "\n";
      out << "\n// Constant pointers for immediate access";
      for (const auto& l : layers) {
         if ( l->getVisible() ) {
            out << "\n#define "<< m_cid <<"_layer_" << nvisl <<"   (u8*)(&layers["<< nvisl <<"][0])";
            ++nvisl;
         }
      }
   }
   out << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate output C-file for basic C conversion (1 layer)
//
void
CPCT_TMX_Tilemap::output_basic_C(std::ostream& out) const {
   output_C_code_header(out);

   // Output tilemap definition
   out << "\n// Generated CSV tilemap from " << m_filename;
   out << "\nconst u8 "<< m_cid <<"["<< m_total_bytes <<"] = {\n";



   out << "\n}\n";
}