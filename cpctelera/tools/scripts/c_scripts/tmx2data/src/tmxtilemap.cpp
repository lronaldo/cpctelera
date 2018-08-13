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
#include "bitarray.h"
#include <helpers.h>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <set>
#include <bitset>
#include <cmath>
#include <ctime>

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap() : m_bitarray(8) { }

CPCT_TMX_Tilemap::CPCT_TMX_Tilemap(char* tmxfilename) : m_bitarray(8) { loadMap(tmxfilename); }

CPCT_TMX_Tilemap::~CPCT_TMX_Tilemap() {
}

void
CPCT_TMX_Tilemap::loadMap(const char* tmxfilename) {
   // Load map and check it to be a correct TMX
   if ( ! m_map.load(tmxfilename) ) error( { "File ", tmxfilename, " does not exist, is not a TMX, is corrupted or could not be loaded." } );

   // Check for required Orientation
   if (m_map.getOrientation() != tmx::Orientation::Orthogonal) error( { "'", tmxfilename, "' has a map orientation different than Orthogonal. Only Orthogonal orientation is supported."} );

   // Check map for at least having 1 visible Layer, and all layers being of type Tile
   // Also count for maxTileID and check that IDs are valid for the m_bitsPerItem setting
   m_visibleLayers = 0;
   m_maxTileID = 0;
   for(const auto& l : m_map.getLayers()) {
      if ( l->getVisible() ) {
         ++m_visibleLayers;

         // Check that any visible layer is of type Tile
         if ( l->getType() != tmx::Layer::Type::Tile ) error ( { "'", tmxfilename, "' contains visible non-tiled layers. Only 'Tile' type layers are supported. " } );

         // Get the Tile layer and the Highest Tile ID (if we are already at 3 digits, there is no need to continue counting)
         updateMaxDecDigits( dynamic_cast<tmx::TileLayer*>(l.get()) );
      }
   }
   // Check that there is at least 1 visible Layer
   if (! m_visibleLayers) error( { "'", tmxfilename, "' has 0 *visible* tilemap layers. At least 1 visible tilemap layer is required. (Remember: only *visible* layers are converted)"} );

   // Check if the maxTileID is valid under the current bitarray settings (there must be enough bits for so many tiles)
   if ( m_maxTileID > m_maxValidTileID ) error( {"The maximum ID of tile used is '", std::to_string(m_maxTileID), "' which requires more bits for its codification than currently selected bitarray settings ('", std::to_string(m_bitarray.getBitsPerItem()), "' bits per tile, Up to ", std::to_string(m_maxValidTileID + 1), " tiles, [0-", std::to_string(m_maxValidTileID), "])." } );

   // Save the filename and time at which the file has been interpreted
   m_filename = tmxfilename;
   std::time_t now = std::time(nullptr);
   m_theTime = std::ctime( &now );
   m_theTime.pop_back();

   // Calculate the parameters of the tilemap
   m_tw = m_map.getTileCount().x;
   m_th = m_map.getTileCount().y;
   m_total_bytes = std::ceil(m_tw * m_th * m_bitarray.getBitsPerItem() / 8.0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate and update max number of decimal digits
//
void           
CPCT_TMX_Tilemap::updateMaxDecDigits(const tmx::TileLayer* l) {
   // Get the Tile layer and the Highest Tile ID (if we are already at 3 digits, there is no need to continue counting)
   if (m_bitarray.getMaxDecDigits() < 3) {
      for( const auto& t : l->getTiles() ) {
         uint8_t v = adjustedTileValue(t.ID);   // Adjust the tile id
         // Calculate the maximum number of decimal digits
         if ( v > m_maxTileID ) {
            m_maxTileID = v;
            if      ( v >= 100 ) { m_bitarray.updateDecimalDigits(3); return; }
            else if ( v >=  10 ) { m_bitarray.updateDecimalDigits(2);         }
         }
      }
   }
}


void
CPCT_TMX_Tilemap::setBitsPerItem(uint8_t bits) {
   m_bitarray.setBitsPerItem(bits);

   // Update bits per item and total bytes
   m_maxValidTileID  = std::pow(2, bits) - 1;
   m_total_bytes     = std::ceil(m_tw * m_th * bits / 8.0);
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
   out << "\n//   * Bits per tile:   " << (uint16_t)m_bitarray.getBitsPerItem();
   out << "\n//   * Layer Bytes:     " << m_total_bytes << " ("<< m_tw <<" x "<< m_th <<" items, "<< (uint16_t)m_bitarray.getBitsPerItem() <<" bits per item)";
   out << "\n//   * Total Bytes:     " << m_total_bytes * m_visibleLayers << " ("<< m_total_bytes <<" x "<< m_visibleLayers <<", bytes per layer times layers)";
   out << "\n//";
   out << "\n#include <types.h>";
   out << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change default C-identifier for output
//
void
CPCT_TMX_Tilemap::setCID(std::string cid) {
   // Check that passed identifier is valid before assigning
   if ( !isValidCIdentifier(cid.c_str()) ) error( { "'", cid, "' is not a valid C-identifier."} );
   m_cid = std::move(cid);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate output header file for basic C conversion
//
void
CPCT_TMX_Tilemap::output_basic_H(std::ostream& out) const {
   output_C_code_header(out);

   // Output declarations
   out << "\n//#### Width and height constants ####";
   out << "\n#define "<< m_cid <<"_W  " << m_tw;
   out << "\n#define "<< m_cid <<"_H  " << m_th;
   out << "\n";
   out << "\n//#### Converted layer tilemaps ####";
   out << "\n//   Visible layers: " << m_visibleLayers;
   out << "\n//";
   if (m_visibleLayers == 1) {
      out << "\nextern const u8 "<< m_cid <<"["<< m_total_bytes <<"];";
   } else {
      out << "\n";
      out << "\n// General layer array";
      out << "\nextern const u8 "<< m_cid <<"["<< m_visibleLayers <<"]["<< m_total_bytes <<"];";
      // Output constants for all layers
      uint16_t nvisl = 0;
      out << "\n";
      out << "\n// Constant pointers for immediate access";
      for (const auto& l : m_map.getLayers()) {
         if ( l->getVisible() ) {
            out << "\n#define "<< m_cid <<"_layer_" << nvisl <<"   (u8*)(&layers["<< nvisl <<"][0])";
            ++nvisl;
         }
      }
   }
   out << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inserts one complete row of tiles in a bitarray.
// If bitarray is not complete after row insertion, some more tiles are inserted to complete the bitarray.
// Bitarray can end up with less than one row if vector of tiles ends previous to bitarray completion.
//
void           
CPCT_TMX_Tilemap::insertOneTileRowInBitarray(CPCT_bitarray& b, TConstItVecTiles it_tiles, TConstItVecTiles it_end) const {
   uint32_t w = m_tw;
   do {
      // Add a complete row if there are enough tiles
      while ( w ) {
         // If there are no more tiles, then immediately end
         if ( it_tiles == it_end ) return;
         // Add next tile ID and count
         b.addItem(adjustedTileValue(it_tiles->ID));
         --w; ++it_tiles;
      }
      // If we have to add more tiles to complete the bitarray, set to add them
      w = b.getItemsToComplete();
   } while (w);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate output C-file for basic C conversion (1 layer)
//
void
CPCT_TMX_Tilemap::output_basic_C(std::ostream& out) const {
   output_C_code_header(out);

   // Output tilemap definition
   out << "\n//#### Converted layer tilemaps ####";
   out << "\n//   Visible layers: " << m_visibleLayers;
   out << "\n//";

   // Variable output depending on visible layers
   std::stringstream ss;
   std::string str_layers = "";
   char c_margin  = ' ';
   char c_prev    = '\0';
   char c_post    = '\0';
   char c_comma   = ' ';
   if (m_visibleLayers > 1) {
      ss << "[" << m_visibleLayers << "]"; str_layers = ss.str(); ss.str("");
      c_prev = '{'; c_post = '}';
   }

   // Output declaration header
   out << "\nconst u8 "<< m_cid << str_layers << "["<< m_total_bytes <<"] = {\n";

   // Output visible layers
   for( const auto& layer : m_map.getLayers() ) {
      // Check if layer is visible (it will be of type Tile if it is visible)
      if ( layer->getVisible() ) {
         const auto& vtiles = dynamic_cast<tmx::TileLayer*>(layer.get())->getTiles();
         TConstItVecTiles it = vtiles.begin();

         out << std::string(3,c_margin) << c_comma << c_prev << '\n';
         char c_sep = ' ';
         while ( it != vtiles.end() ) {
            m_bitarray.reset();
            insertOneTileRowInBitarray(m_bitarray, it, vtiles.end()); 
            if ( m_bitarray.getNumItems() ) {
               out << std::string(3,c_margin) << c_sep;
               m_bitarray.print(out);
               out << '\n';
               it = it + (uint32_t)m_bitarray.getNumItems();
               c_sep = ',';
            }
         }
         out << std::string(3,c_margin) << c_post << '\n';
         c_comma = ',';
      }
   }

   // Output declaration end
   out << "};\n";
}
