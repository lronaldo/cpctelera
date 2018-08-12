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

#pragma once

#include <tmxlite/Map.hpp>
#include <string>
#include <iostream>

class CPCT_TMX_Tilemap {
public:
   enum class NumberFormat {
         decimal
      ,  hexadecimal
      ,  binary
   };

	CPCT_TMX_Tilemap();
   CPCT_TMX_Tilemap(char* tmxfilename);
   ~CPCT_TMX_Tilemap();

   void  loadMap(const char* tmxfilename);
   void  setBitsPerItem(uint8_t bits);
   void  setOutputNumberFormat(NumberFormat f)  { m_numFormat = f;      }
   void  setInitialTileID(uint8_t id)           { m_initialTileID = id; }

   void  printSomeInfo() const;

   // Output operators
   void  output_C_code_header(std::ostream& out) const;
   void  output_basic_H(std::ostream& out) const;
   void  output_basic_C(std::ostream& out) const;
private:
   // Attributes
   std::string    m_filename;             // Name of the TMX input file
   std::string    m_cid = "g_map";        // C-identifier for output
   tmx::Map       m_map;                  // TMX interpreter
   uint8_t        m_bitsPerItem = 8;      // Number of bits per tile
   uint8_t        m_maxTileID;            // Highest ID amongst tiles
   uint8_t        m_maxTileDecDigits;     // Number of decimal digits of the Highest tile ID
   uint8_t        m_initialTileID = 0;    // This will be considered the ID of the first tile
   uint32_t       m_visibleLayers;        // Number of visible layers in the TMX
   uint32_t       m_tw, m_th;             // Tile Width and Height
   uint32_t       m_total_bytes;          // Total bytes for a given output array
   std::string    m_theTime;              // Current time string
   NumberFormat   m_numFormat = NumberFormat::decimal;  // Output number format selected


   // Useful methods
   void           output_formatted_tile(std::ostream& out, uint8_t tile) const;
   inline uint8_t adjustedTileValue(uint8_t tile) const { 
      if (tile) return tile - 1 + m_initialTileID;
      else      return tile + m_initialTileID;
   }
};