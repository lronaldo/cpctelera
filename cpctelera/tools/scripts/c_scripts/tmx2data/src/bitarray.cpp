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

#include "bitarray.h"
#include "helpers.h"
#include <cmath>
#include <iomanip>
#include <bitset>

////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor of the class
// It will throw a std::runtime exception if the number of bits is not supported
//
CPCT_bitarray::CPCT_bitarray(uint8_t bits) {
   setBitsPerItem(bits);
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Add a new item to the list of items to be converted. 
// Throws a runtime exception if item cannot be added because of it being out of bounds
//
void 
CPCT_bitarray::addItem(uint8_t item) {
   // Check boundaries before adding
   if ( item > m_maxItemValue ) error( { "Value '", std::to_string(item), "' is outside valid boundaries for current bitarray settings ('", std::to_string(m_bitsPerItem), "' bits per item, Valid range [0-", std::to_string(m_maxItemValue), "])." } );
   m_vecItems.push_back(item);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the bits per item that will handle this bitarray
// It will throw a std::runtime exception if the number of bits is not supported
//
void
CPCT_bitarray::setBitsPerItem(uint8_t bits) {
   std::set<uint8_t> validbits { 1, 2, 4, 6, 8 };

   // Check that the given number of bits is valid
   if (validbits.find(bits) == validbits.end()) error( { "Invalid number of bits per item '", std::to_string(bits), "'. Valid values are: ", setToString(validbits, " ") } );

   // Update bits per item and total bytes
   m_bitsPerItem  = bits;
   m_maxItemValue = std::pow(2, bits) - 1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints an isolated item in the currently selected format
//
void
CPCT_bitarray::printItem(std::ostream& out, uint8_t item) const {
   uint32_t pitem = item;  // Convert to 32bits to print it as number
   switch(m_outFormat) {
      case TNumberFormat::decimal:     { out << std::setw(m_maxDecDigits) << std::setbase(10) << pitem;                   break; }
      case TNumberFormat::binary_text: { std::bitset<8> bs(pitem); out << "0b" << std::setw(8) << bs;                     break; }
      case TNumberFormat::binary:      { out.put(item);                                                                   break; }
      case TNumberFormat::hexadecimal: { out << "0x" << std::setw(2) << std::setfill('0') << std::setbase(16) << pitem;   break; }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints output values in the form of a CPCtelera macro.
// Uses m_CPCTMacroName as name of the macro to be output
//
void
CPCT_bitarray::printCPCTMacro(std::ostream& out, const TVecItems& v) const {
   char c_sep = '\0';
   out << m_CPCTMacroName << "(";
   for( const auto& i : v ) {
      printSeparator(out, c_sep);
      printItem(out, i);
      c_sep = m_separator;
   }
   out << ")";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints a separator character depending on current output format
//
void
CPCT_bitarray::printSeparator(std::ostream& out, char c_sep) const {
   if ( m_outFormat != TNumberFormat::binary && c_sep != '\0' ) out << c_sep;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print output values in groups of 4 items, using 6 bits per item
// Throws a std::runtime exception if size of v is incorrect
//
void
CPCT_bitarray::printGroupOf4_6bits(std::ostream& out, const TVecItems& v) const {
   if ( v.size() != 4 ) error( { "Error converting to bitarrays of 6 bits. A group of '", std::to_string(v.size()) ,"' items was received when expecting 4 items." } );

   // Print 3 bytes formed with this 4 items
   printItem(out, (v[0] << 2) | ((v[1]       ) >> 6) ); printSeparator(out, m_separator);
   printItem(out, (v[1] << 4) | ((v[2] & 0x3F) >> 2) ); printSeparator(out, m_separator);
   printItem(out, (v[2] << 6) | ((v[3] & 0x3F)     ) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print output values in groups of 2 items, using 4 bits per item
// Throws a std::runtime exception if size of v is incorrect
//
void
CPCT_bitarray::printGroupOf2_4bits(std::ostream& out, const TVecItems& v) const {
   if ( v.size() != 2 ) error( { "Error converting to bitarrays of 4 bits. A group of '", std::to_string(v.size()) ,"' items was received when expecting 2 items." } );

   // Print 1 byte formed with this 2 items
   printItem(out, (v[0] << 4) | (v[1] & 0x0F) ); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print output values in groups of 4 items, using 2 bits per item
// Throws a std::runtime exception if size of v is incorrect
//
void
CPCT_bitarray::printGroupOf4_2bits(std::ostream& out, const TVecItems& v) const {
   if ( v.size() != 4 ) error( { "Error converting to bitarrays of 2 bits. A group of '", std::to_string(v.size()) ,"' items was received when expecting 4 items." } );

   // Print 1 byte formed with this 4 items
   printItem(out, (v[0] << 6) | ((v[1] & 1) << 4) | ((v[2] & 1) << 2) | (v[3] & 1) ); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print output values in groups of 8 items, using 1 bits per item
// Throws a std::runtime exception if size of v is incorrect
//
void
CPCT_bitarray::printGroupOf8_1bits(std::ostream& out, const TVecItems& v) const {
   if ( v.size() != 8 ) error( { "Error converting to bitarrays of 1 bit. A group of '", std::to_string(v.size()) ,"' items was received when expecting 8 items." } );

   // Print 1 byte formed with this 4 items
   printItem(out,    (v[0]      << 7) | ((v[1] & 1) << 6) | ((v[2] & 1) << 5) | ((v[3] & 1) << 4)
                  | ((v[4] & 1) << 3) | ((v[5] & 1) << 2) | ((v[6] & 1) << 1) |  (v[7] & 1)      ); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forms groups of items and prints them one by one calling callback method
//
void
CPCT_bitarray::printInGroups(std::ostream& out, uint32_t n, TPrintGroupMethod printMethod) const { 
   char c_sep = '\0';

   // Produce all possible complete groups (size n) and leave remaining items for next pass
   uint32_t g = m_vecItems.size() / n;   // Number of complete groups (integer division)
   auto pinit = m_vecItems.begin();      // Initial pointer (for a group)
   auto pend  = m_vecItems.begin() + n;  // Ending pointer
   for(uint32_t i=0; i < g; ++i, pinit += n, pend += n) {
      TVecItems v(pinit, pend);
      printSeparator(out, c_sep); (this->*printMethod)(out, v);
      c_sep = m_separator;
   }

   // If there are remaining items, produce the last group call
   uint32_t m = m_vecItems.size() % n;    // Remaining items
   if ( m ) {
      TVecItems v(pinit, m_vecItems.end());
      v.insert(v.end(), m, 0);            // Insert 0's to complete a group
      printSeparator(out, c_sep); (this->*printMethod)(out, v);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints all the items in the vector individually as a stream
//
void
CPCT_bitarray::printIndividually(std::ostream& out) const { 
   char c_sep = '\0';

   // Print all items individually
   for ( const auto& i : m_vecItems ) {
      printSeparator(out, c_sep); printItem(out, i);
      c_sep = m_separator;
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints the bitarray to a stream
//
void
CPCT_bitarray::print(std::ostream& out) const {
   // Ensure valid value for maxDecDigits
   if ( m_outFormat == TNumberFormat::decimal ) {
      if ( m_bitsPerItem < 8 && !m_useCPCTMacros ) m_maxDecDigits = 3;
   }

   // Printing depends on bits per item
   if ( m_bitsPerItem == 8 ) printIndividually(out);  // No need of groups or macros when individual
   else {
      // Select number of items per group and printing method
      uint32_t          n = 0;
      TPrintGroupMethod p = nullptr;
      switch( m_bitsPerItem ) {
         case 1:  { n = 8; p = &CPCT_bitarray::printGroupOf8_1bits; break; }
         case 2:  { n = 4; p = &CPCT_bitarray::printGroupOf4_2bits; break; }
         case 4:  { n = 2; p = &CPCT_bitarray::printGroupOf2_4bits; break; }
         case 6:  { n = 4; p = &CPCT_bitarray::printGroupOf4_6bits; break; }
      }
      // When CPCTMacros are in use, call method printCPCTMacro
      if ( m_useCPCTMacros && m_outFormat != TNumberFormat::binary && m_bitsPerItem > 1 ) 
         p = &CPCT_bitarray::printCPCTMacro;
      
      // Call for group printing
      printInGroups(out, n, p);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns number of items that remain to be inserted to have a complete, ready, per groups output
//
uint8_t     
CPCT_bitarray::getItemsToComplete() const {
   uint8_t m = 1;
   switch ( m_bitsPerItem ) {
      case 1: m = 8; break;
      case 2: m = 4; break;      
      case 4: m = 2; break;      
      case 6: m = 4; break;      
   }
   return m_vecItems.size() % m;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the output format that will be used 
// Depending on the output format, some other values may be assigned accordingly for correct visualization
//
void
CPCT_bitarray::setOutFormat(TNumberFormat f) { 
   m_outFormat = f;
}