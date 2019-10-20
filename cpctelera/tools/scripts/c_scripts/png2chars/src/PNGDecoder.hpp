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

#include <vector>
#include <cstdint>
#include <array>

using TVecUchar      = std::vector<unsigned char>;
using TArrayChar8x8  = std::array<unsigned char, 8>;

struct PNGDecoder {
   enum Generate {
         _C  = 0x01
      ,  _S  = 0x02
      ,  _HS = 0x04
      ,  _H  = 0x08
      ,  _BIN= 0x10
      ,  _BAS= 0x20
      ,  _DEF= 0x80
   };

   explicit PNGDecoder();
   void readFile(std::string filename);
   void startConversion() const;
   TArrayChar8x8 convertNextCharacter() const;
   void drawPNGCharacter8x8(const TArrayChar8x8& chardef) const;
   void outputCharDefsBASIC(const TArrayChar8x8& chardef, uint16_t symbol, uint16_t line) const;
   void outputCharDefsCArrayData(const TArrayChar8x8& chardef, uint16_t charnum, char comma) const;
   void outputCharDefsASMData(const TArrayChar8x8& chardef, uint16_t charnum) const;
   void convertAllCharsToBASIC() const;
   void convertAllCharsToC() const;
   void convertAllCharsToASM() const;
   void convert() const;
   void drawPNGCharByChar() const;
   void setCArrayName(std::string cid) { m_CArrayName = cid; }
   void setIdentifierPrefix(char p) { m_idPrefix = '_'; }
   void setASMConstantLocal(bool l) { m_asmConstantLocal = l; }
   void setGenerate(uint8_t gen) { 
      if ( m_generate & _DEF ) m_generate = gen;   // Remove defaults
      else m_generate |= gen;                      // Add new value, defaults are already removed
   }   

private:
   TVecUchar         m_image;
   uint64_t          m_width {0}, m_height {0};
   uint16_t          m_basicLine {10}, m_basicSymbol {32};
   std::string       m_CArrayName {"chardata"};
   char              m_idPrefix {' '};
   bool              m_asmConstantLocal { false };
   uint8_t           m_generate { _DEF | _C | _H };
   mutable uint64_t  m_convWidth {0}, m_convHeight {0};
};

