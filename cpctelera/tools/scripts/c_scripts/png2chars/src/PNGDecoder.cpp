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

#include <PNGDecoder.hpp>
#include <picopng.hpp>
#include <helpers.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <array>
#include <iomanip>


PNGDecoder::PNGDecoder() {
}

void 
PNGDecoder::readFile(std::string filename) {
   // Read PNG file into memory
   std::ifstream fontfile(filename, std::ios::binary);
   if ( !fontfile ) error({"File '", filename, "' could not be opened."});  
   TVecUchar filebuffer(std::istreambuf_iterator<char>(fontfile), {});

   // Decode PNG into memory
   int err = decodePNG(m_image, m_width, m_height, filebuffer.data(), filebuffer.size());
   if ( err ) error({ "There was an error decoding file '", filename, "' as PNG file." });

   // Check that file is multiple of 8x8
   if ( m_width  & 7 ) error({"Image width (", std::to_string(m_width), ") is not a multiple of 8."});
   if ( m_height & 7 ) error({"Image height (", std::to_string(m_height), ") is not a multiple of 8."});
}

void
PNGDecoder::startConversion() const {
   // TODO: Check m_image is valid
   m_convWidth = m_convHeight = 0;
}

TArrayChar8x8
PNGDecoder::convertNextCharacter() const {
   TArrayChar8x8 chardef;

   // If coordinates outside image, return blank character
   if (m_convWidth >= m_width || m_convHeight >= m_height)
      return chardef;

   // Set pointer to current widht and height
   const unsigned char* pixels = m_image.data();
   pixels +=   m_convHeight * m_width*4
            +  m_convWidth * 4;

   // Perform conversion
   for (uint16_t i=0; i < 8;  ++i) {
      unsigned char row { 0 };
      for (uint16_t j=0; j < 8; ++j) { 
         uint16_t val = *pixels + *(pixels+1) + *(pixels+2);
         row <<= 1;
         if ( val ) row |= 1;
         pixels += 4;
      }
      chardef[i] = row;
      pixels += 4 * (m_width - 8);
   }

   // Update Coordinates for next conversion
   m_convWidth += 8;
   if ( m_convWidth >= m_width ) {
      m_convWidth = 0;
      m_convHeight += 8;
   }

   // Return character definition
   return chardef;
}

void
PNGDecoder::drawPNGCharacter8x8(const TArrayChar8x8& chardef) const {
   // Get first 8x8 character and draw it
   std::cout << "/--------\\\n";
   for (auto row : chardef) {
      std::cout << "|";
      for(uint8_t i=0; i < 8; ++i) {
         std::cout << ((row & 0x80) ? '#' : ' ');
         row <<= 1;
      }
      std::cout << "|\n";
   }
   std::cout << "\\--------/\n";
}

void
PNGDecoder::outputCharDefsBASIC(const TArrayChar8x8& chardef, uint16_t symbol, uint16_t line) const {
   std::cout << line << " SYMBOL " << symbol;
   for (auto c : chardef) {
      std::cout << "," << uint16_t(c);
   }
   std::cout << "\n";
}

void
PNGDecoder::outputCharDefsCArrayData(const TArrayChar8x8& chardef, uint16_t charnum, char comma) const {
   uint16_t size = chardef.size();

   std::cout << " /* " << std::setw(3) << charnum << " */ " << comma;
   std::cout << std::hex << std::setfill('0');
   for (uint8_t i=0; i < size-1; ++i) {
      std::cout << "0x" << std::setw(2) 
                << uint16_t(chardef[i]) << ",";
   }
   std::cout   << "0x" << std::setw(2)
               << uint16_t(chardef[size-1]) << "\n";
   std::cout << std::dec << std::setfill(' ');
}

void
PNGDecoder::outputCharDefsASMData(const TArrayChar8x8& chardef, uint16_t charnum) const {
   uint16_t size = chardef.size();

   std::cout << " .db ";
   std::cout << std::hex << std::setfill('0');
   for (uint8_t i=0; i < size-1; ++i) {
      std::cout << "0x" << std::setw(2) 
                << uint16_t(chardef[i]) << ",";
   }
   std::cout   << "0x" << std::setw(2)
               << uint16_t(chardef[size-1]);
   std::cout << std::dec << std::setfill(' ');
   std::cout << "  ;; " << std::setw(3) << std::dec << charnum << "\n";
}

void
PNGDecoder::convertAllCharsToBASIC() const {
   uint16_t sym  = m_basicSymbol;
   uint16_t line = m_basicLine;
   uint32_t numchars = m_width*m_height/64;

   // Convert characters to BASIC
   startConversion();
   while(numchars--) {
      outputCharDefsBASIC( convertNextCharacter(), sym, line );
      sym ++;
      line += 10;
   }
}

void
PNGDecoder::convertAllCharsToC() const {
   uint16_t sym = m_basicSymbol;
   uint32_t numchars = m_width*m_height/64;

   // Convert characters to C
   startConversion();
   std::cout << m_CArrayName << "[" << m_width << "*" << m_height << "] = {\n";
   outputCharDefsCArrayData( convertNextCharacter(), sym, ' ');
   for (uint32_t i=1; i < numchars; ++i) {
      ++sym;
      outputCharDefsCArrayData( convertNextCharacter(), sym, ',');
   }
   std::cout << "};\n";
}

void
PNGDecoder::convertAllCharsToASM() const {
   uint16_t sym = m_basicSymbol;
   uint32_t numchars = m_width*m_height/64;

   // Convert characters to C
   startConversion();
   if (m_idPrefix != ' ') std::cout << m_idPrefix;
   std::cout << m_CArrayName << ":";
   if (!m_asmConstantLocal) std::cout << ":";
   std::cout << "\n";
   while(numchars--) {
      outputCharDefsASMData( convertNextCharacter(), sym);
      ++sym;
   }
}

void 
PNGDecoder::convert() const {
   if ( m_generate & _C    ) convertAllCharsToC();
   if ( m_generate & _S    ) convertAllCharsToASM();
   if ( m_generate & _BAS  ) convertAllCharsToBASIC();
}

void
PNGDecoder::drawPNGCharByChar() const {
   uint32_t numchars = m_width*m_height/64;

   // Draw characters
   startConversion();
   while (numchars--) {
      drawPNGCharacter8x8( convertNextCharacter() );
      std::string kk;
      std::cin >> kk;
   }
}
