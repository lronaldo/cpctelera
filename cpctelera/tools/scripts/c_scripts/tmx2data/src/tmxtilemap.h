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
#include <chrono>
#include <ctime>
#include <iostream>

class CPCT_TMX_Tilemap {
public:
	CPCT_TMX_Tilemap();
   CPCT_TMX_Tilemap(char* tmxfilename);
   ~CPCT_TMX_Tilemap();

   void  loadMap(const char* tmxfilename);
   void  setBitsPerItem(uint8_t bits);

   void  printSomeInfo() const;

   // Output operators
   void  output_basic_H(std::ostream& out) const;
private:
   std::string m_filename;
   std::string m_cid = "g_map";
   tmx::Map    m_map;
   uint8_t     m_bitsPerItem = 8;
   std::time_t m_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
};