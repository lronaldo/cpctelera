//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#include <conversors/RGBAConversor.hpp>
#include <conversors/CConversor.hpp>
#include <conversors/HConversor.hpp>
#include <conversors/HSConversor.hpp>
#include <conversors/ASMConversor.hpp>
#include <conversors/BASICConversor.hpp>
#include <conversors/BINConversor.hpp>
#include <conversors/TerminalTestDrawConversor.hpp>
#include <helpers.h>

#include <vector>
#include <cstdint>
#include <array>
#include <memory>
#include <functional>

// Forward declarations
struct RGBAConversor;

using TVecUchar      = std::vector<unsigned char>;
using TArrayChar8x8  = std::array<unsigned char, 8>;

struct PNGDecoder {
   // Total number of conversors
   static constexpr uint16_t knum_conversors { 7 };

   // Types
   enum Generate {
         GN_C    = 0x01
      ,  GN_S    = 0x02
      ,  GN_HS   = 0x04
      ,  GN_H    = 0x08
      ,  GN_BIN  = 0x10
      ,  GN_BAS  = 0x20
      ,  GN_DRAW = 0x40
      ,  GN_DEF  = 0x80
   };

   struct ConversorPack {
      std::unique_ptr<RGBAConversor> conv;
      PNGDecoder::Generate flag;
      std::string ext;
      std::string formatName;
   };
   using TArrayConversors  = std::array<ConversorPack, knum_conversors>;
   
   // Explicit initialization
   explicit PNGDecoder();

   // Setters
   void setForAll(std::function<void(ConversorPack&)>&& f);
   void setCArrayName(std::string cid) {
      setForAll( [cid](ConversorPack& c) { c.conv->setCIdentifier(cid.c_str()); });
   }
   void setIdentifierPrefix(char p) { 
      setForAll( [p](ConversorPack& c) { c.conv->setCharPrefix(p); });
   }
   void setASMConstantLocal(bool l) {
      setForAll( [l](ConversorPack& c) { c.conv->setLocalConstant(l); });
   }
   void setOutputNumberFormat(TNumberFormat nf) {
      setForAll( [nf](ConversorPack& c) { c.conv->setNumberFormat(nf); });
   }

   void setGenerate(uint8_t gen) { 
      if ( m_generate & GN_DEF ) m_generate = gen;   // Remove defaults
      else m_generate |= gen;                        // Add new value, defaults are already removed
   }   

   // Conversion functions
   void readFile(std::string filename);
   void convert(std::string& outputFolder, std::string& fileBaseName) const;

private:
   TVecUchar         m_image;
   long unsigned     m_width {0}, m_height {0};
   uint8_t           m_generate { GN_DEF | GN_C | GN_H };
   TArrayConversors  m_conversors{};
   std::string       m_filename{};
};
