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
	CPCT_TMX_Tilemap();
   CPCT_TMX_Tilemap(char* tmxfilename);
   ~CPCT_TMX_Tilemap();

   void  loadMap(const char* tmxfilename);
   void  setBitsPerItem(uint8_t bits);

   void  printSomeInfo() const;

   // Check and setup for output
   void  checkAndSetUpForOutput();

   // Output operators
   void  output_C_code_header(std::ostream& out) const;
   void  output_basic_H(std::ostream& out) const;
   void  output_basic_C(std::ostream& out) const;
private:
   std::string m_filename;
   std::string m_cid = "g_map";
   tmx::Map    m_map;
   uint8_t     m_bitsPerItem = 8;
   uint16_t    m_visibleLayers;  // Number of visible layers in the TMX
   uint32_t    m_tw, m_th;       // Tile Width and Height
   uint32_t    m_total_bytes;    // Total bytes for a given output array
   std::string m_theTime;        // Current time string
};