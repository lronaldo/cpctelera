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

#include <cstdint>
#include <string>
#include <fstream>

namespace CPCT {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PATCHER CLASS
//
//    Encapsulates all methods for patching a given file
//
struct Patcher {
   // Distinguish between endianness types
   enum class TEndian { little, big };

   Patcher();
   ~Patcher();

   void reset();
   void patchByte(uint32_t offset, char byte);
   void patchWord(uint32_t offset, uint16_t word);
   void patchStream(uint32_t offset, const std::string& stream);
   void setBinFile(std::string filename);
   void setEndianess(TEndian e) { m_endianess = e; }
   void saveFile();

   void print(std::ostream& out) const;
   const std::string& filename() { return m_filename; }

private:
   // Constants
   static const uint32_t M_MAXSIZE  = 8 * 1024 * 1024; // Up to 8MB

   // Status Attributes
   std::fstream   m_fs;
   std::string    m_filename;
   char*          m_theFile   = nullptr;
   uint32_t       m_fileSize  = 0;
   uint32_t       m_totalSize = 0;

   // Configuration Attributes (Do not change on reset())
   TEndian        m_endianess = TEndian::little;

   void readFileIntoBuffer();
   void calculateFileSize();
   void reserveNewFileBuffer();
   void updateTotalSize(uint32_t addSize);
};

} // End Namespace CPCT