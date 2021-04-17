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
#include <tmxlite/TileLayer.hpp>
#include <string>
#include <iostream>
#include "outFormat.h"
#include "bitarray.h"

class CPCT_TMX_Tilemap {
public:
   // Type aliases
   using TConstItVecTiles = std::vector<tmx::TileLayer::Tile>::const_iterator;
   using TVecTiles        = std::vector<tmx::TileLayer::Tile>;

   // Constants
   const char* k_macro2bits = "CPCT_ENCODE2BITS";
   const char* k_macro4bits = "CPCT_ENCODE4BITS";
   const char* k_macro6bits = "CPCT_ENCODE6BITS";

   // Constructors
	CPCT_TMX_Tilemap();
   CPCT_TMX_Tilemap(char* tmxfilename);
   ~CPCT_TMX_Tilemap();

   void  loadMap(const char* tmxfilename);
   void  setBitsPerItem(uint8_t bits);
   void  setCID(std::string cid);
   void  setOutputNumberFormat(TNumberFormat f) noexcept { m_bitarray.setOutFormat(f);       }
   void  setInitialTileID(uint8_t id)           noexcept { m_initialTileID = id;             }
   void  setUseCPCTMacros(bool use)             noexcept { m_bitarray.setUseCPCTMacros(use); }
   void  setASMVariablesPrefix(char p)          noexcept { m_asmVarsPrefix = p;              }
   void  setASMConstantsLocal(bool b)           noexcept { m_asmConstantsLocal = b;          }
   void  setASMGenerateGlobal(bool g)           noexcept { m_asmGenerateGlobal = g;          }

   // Output operators
   void  output_basic_H  (std::ostream& out) const;
   void  output_basic_HS (std::ostream& out) const;
   void  output_basic_S  (std::ostream& out) const;
   void  output_basic_C  (std::ostream& out) const;
   void  output_basic_BIN(std::ostream& out) const;

   // Getters
   TNumberFormat getNumberFormat()     { return m_bitarray.getNumberFormat(); }
private:
   // Attributes
   mutable CPCT_bitarray  m_bitarray;           // Class that encapsulate conversion from bitarray numbers to C/H/BIN
   std::string    m_filename;                   // Name of the TMX input file
   std::string    m_cid = "g_map";              // C-identifier for output
   tmx::Map       m_map;                        // TMX interpreter
   uint8_t        m_maxTileID;                  // Highest ID amongst tiles
   uint8_t        m_initialTileID = 0;          // This will be considered the ID of the first tile
   uint8_t        m_maxValidTileID = 255;       // Max valid Tile ID number depending on bits per item
   uint32_t       m_visibleLayers;              // Number of visible layers in the TMX
   uint32_t       m_tw = 0, m_th = 0;           // Tile Width and Height
   uint32_t       m_total_bytes;                // Total bytes for a given output array
   std::string    m_theTime;                    // Current time string
   char           m_asmVarsPrefix = 0;          // Prefix for assembly variable names (Default: none)
   bool           m_asmConstantsLocal = true;   // Decide if assembly generated constants have to be local or global
   bool           m_asmGenerateGlobal = true;   // Decide if generated assembly header contains a .globl declaration for each label in the source file

   // Useful methods
   void           output_asmVar(std::ostream& out, const std::string& var) const;
   void           output_C_code_header(std::ostream& out, const char* comm) const;
   void           updateTotalBytes();
   void           updateMaxDecDigits(const tmx::TileLayer* l);
   void           insertOneTileRowInBitarray(CPCT_bitarray& b, TConstItVecTiles it_tiles, TConstItVecTiles it_end) const;
   inline uint8_t adjustedTileValue(uint8_t tile) const { 
      if (tile) return tile - 1 + m_initialTileID;
      else      return tile + m_initialTileID;
   }
};