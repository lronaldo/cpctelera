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

#include <cstdint>
#include <vector>
#include <string>
#include "outFormat.h"

////////////////////////////////////////////////////////////////////////////////////////////////
// Class to give support to transforming bitarrays into output values. Conversion produces an
// output string, typically one row, in the selected output format
//
class CPCT_bitarray {
public:
   // Types
   using TVecItems = std::vector<uint8_t>;
   using TPrintGroupMethod = void (CPCT_bitarray::*)(std::ostream&, const TVecItems&) const;

   // Constructors
   CPCT_bitarray(uint8_t bits);

   // Methods
   void        addItem(uint8_t item);
   void        setSeparator(char c)             { m_separator  = c;                 }
   void        setDecimalDigits(uint8_t d)      { m_maxDecDigits = d;               }
   void        setOutFormat(TNumberFormat f);
   void        setUseCPCTMacros(bool use)       { m_useCPCTMacros = use;            }
   void        setCPCTMacroName(std::string mn) { m_CPCTMacroName = std::move(mn);  }
   void        setBitsPerItem(uint8_t bits);
   void        updateDecimalDigits(uint8_t d)   { if ( d > m_maxDecDigits ) m_maxDecDigits = d;  }

   void        print(std::ostream& out) const;
   void        reset()                          { m_vecItems.clear();               }

   std::size_t getNumItems()        const { return m_vecItems.size();   }
   uint8_t     getMaxItemValue()    const { return m_maxItemValue;      }
   uint8_t     getMaxDecDigits()    const { return m_maxDecDigits;      }
   uint8_t     getBitsPerItem()     const { return m_bitsPerItem;       }
   bool        usingCPCTMacros()    const { return m_useCPCTMacros;     }
   std::string getCPCTMacroName()   const { return m_CPCTMacroName;     }
   uint8_t     getItemsToComplete() const;
   TNumberFormat getNumberFormat()  const { return m_outFormat;         }
private:
   bool        m_useCPCTMacros = true; // Use CPCTelera bitarray macros for all text conversions
   uint8_t     m_bitsPerItem = 8;      // Bits per item in the bitarray
   uint8_t     m_maxItemValue = 255;   // Maximum value for an item
   mutable uint8_t     m_maxDecDigits = 1; // Number of decimal digits for the maximum value
   TVecItems   m_vecItems;             // Vector of items in the bitarray to be transformed
   TNumberFormat m_outFormat = TNumberFormat::decimal; // Output format for conversion
   std::string m_CPCTMacroName = "_B"; // Macro name that will be used for brevity instead of the corresponding CPCT_MACRO
   char        m_separator = ',';      // Separator character in between items

   // Private methods
   void        printItem(std::ostream& out, uint8_t item) const;
   void        printIndividually(std::ostream& out) const;
   void        printInGroups(std::ostream& out, uint32_t n, TPrintGroupMethod) const;
   void        printCPCTMacro(std::ostream& out, const TVecItems& v) const;
   void        printGroupOf4_6bits(std::ostream& out, const TVecItems& v) const;
   void        printGroupOf2_4bits(std::ostream& out, const TVecItems& v) const;
   void        printGroupOf4_2bits(std::ostream& out, const TVecItems& v) const;
   void        printGroupOf8_1bits(std::ostream& out, const TVecItems& v) const;
   void        printSeparator(std::ostream& out, char c_sep) const;

};
