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

#include <iomanip>
#include <iostream>
#include <patcher.h>
#include <helpers.h>

namespace CPCT {

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//
Patcher::Patcher() {}
   
Patcher::~Patcher() {
   reset();
}
   
///////////////////////////////////////////////////////////////////////////////
// RESET
//    Resets the status of the Patcher object, and leaves it ready for a new
// file to be patched.
//
void 
Patcher::reset() {
   if (m_fs) m_fs.close();
   m_filename  = "";
   m_fileSize  = 0;
   m_totalSize = 0;
   SAFEDELETE(m_theFile);
}

///////////////////////////////////////////////////////////////////////////////
// CALCULATE FILE SIZE
//    Once the file has been set, it calculates the size of the file.
// THROWS: runtime_error
//
void
Patcher::calculateFileSize() {
   // Open and check it opened correctly
   m_fs.open(m_filename, std::ios::in | std::ios::ate );
   if (!m_fs) error( { "File '", m_filename, "' could not be opened. It either does not exist or your user has not enough privileges." } );

   // Calculate size and close
   m_fileSize = m_fs.tellg();
   m_fs.close();      
}

///////////////////////////////////////////////////////////////////////////////
// RESERVE NEW FILE BUFFER
//    Reserves a new file buffer to store the entire file in memory. As the
// buffer is new, it removes previously existing buffers, if there were any.
// THROWS: bad_alloc
//
void
Patcher::reserveNewFileBuffer() {
   // Remove previous buffer
   SAFEDELETE(m_theFile);

   // Update total size and reserve new buffer 
   updateTotalSize(m_fileSize);
   m_theFile = new char[m_fileSize];
}

///////////////////////////////////////////////////////////////////////////////
// READ FILE INTO BUFFER
//    Reads a given file and loads it completely into a new buffer. As it is
// a new file, it resets previous buffers, including those for appending
// files at the end, and resets total bytes counter.
// THROWS: bad_alloc, runtime_error
//
void
Patcher::readFileIntoBuffer() {
   // Before reading a new file into buffer, its size needs to be calculated
   calculateFileSize();

   // Open file for binary read (and check it opened correctly)
   m_fs.open(m_filename, std::ios::in | std::ios::binary);      
   if (!m_fs) error( { "File '", m_filename, "' could not be openned." } );
   
   // Reserve a file buffer and read the full file into it
   reserveNewFileBuffer();
   m_fs.read(m_theFile, m_fileSize);

   // Close the file
   m_fs.close();
}

///////////////////////////////////////////////////////////////////////////////
// PATCH BYTE
//    It changes the value of a given BYTE at OFFSET
// THROWS: runtime_error
//
void
Patcher::patchByte(uint32_t offset, char byte) {
   // Check for offset being outside file boundaries
   if (offset >= m_fileSize) error ( { "Given offset '", std::to_string(offset), "' is out of bounds. File range is [0, ", std::to_string(m_fileSize-1),"]." } );

   // Patch the byte into the file
   m_theFile[offset] = byte;
}

///////////////////////////////////////////////////////////////////////////////
// PATCH WORD
//    It changes the value of a given WORD at OFFSET, taking into account
// currently selected endianess (little endian, by default)
// THROWS: runtime_error
//
void
Patcher::patchWord(uint32_t offset, uint16_t word) {
   // Check for offset being outside file boundaries
   if (offset + 1 >= m_fileSize) error ( { "Given offset '", std::to_string(offset), "' for a word (2 bytes) is out of bounds. File size is: '", std::to_string(m_fileSize),"'." } );
   
   // In case of big endian, invert byte order in the word
   if ( m_endianess == TEndian::big )
      word = (word >> 8) | (word << 8);

   // Patch both bytes into the file
   m_theFile[offset  ] = static_cast<char>(word & 0xFF);
   m_theFile[offset+1] = static_cast<char>(word >> 8);
}

///////////////////////////////////////////////////////////////////////////////
// PATCH STREAM
//    It changes a complete STREAM of bytes starting at OFFSET, byte by byte.
// The stream must fit into the current binfile size.
//
// THROWS: runtime_error
//
void
Patcher::patchStream(uint32_t offset, const std::string& stream) {
   // Check for offset being outside file boundaries
   if (offset + stream.size() >= m_fileSize) error ( { "Cannot patch byte stream of '", std::to_string(stream.size()), "' bytes at offset '", std::to_string(offset), "'. It would reach offset '", std::to_string(stream.size() + offset), "', which would be out of bounds. File size is: '", std::to_string(m_fileSize),"'." } );

   // Patch the stream byte by byte
   for (auto &c : stream) {
      m_theFile[offset] = c;
      ++offset;
   }
}


///////////////////////////////////////////////////////////////////////////////
// SET BIN FILE
//    Sets a new binary file to be patched. It removes any previous file and
// resets Patcher status before setting the new file. Therefore, using this 
// method is similar to restarting the patcher completely with a new file.
// After resetting the Patcher, it reads the new file into memory.
// THROWS: bad_alloc, runtime_error
//
void
Patcher::setBinFile(std::string filename) {
   // Reset the Patcher
   reset();

   // Set the new file and read it into memory
   m_filename = std::move(filename);
   readFileIntoBuffer();
}

///////////////////////////////////////////////////////////////////////////////
// UPDATE TOTAL SIZE
//    Updates the total size of the output binary forming. This total size
// includes the input binary file and the modifications that append new binary
// data at the end of the file. If any update to the total size makes it grow
// beyond the size limit, a runtime_error is thrown.
// THROWS: runtime_error
//
void 
Patcher::updateTotalSize(uint32_t addSize) {
   // Update size and check limits
   m_totalSize += addSize;
   if ( m_totalSize > M_MAXSIZE ) error ( { "[PATCHER]: Total size of patched binary exceeds maximum binary size ('", std::to_string(m_totalSize) ,"' > '", std::to_string(M_MAXSIZE),"')." } );
}

///////////////////////////////////////////////////////////////////////////////
// SAVE FILE
//    Saves the file after being patched. It overwrites the original file
// with its new values.
// THROWS: runtime_error
//
void
Patcher::saveFile() {
   // Open file for binary write (and check it opened correctly)
   m_fs.open(m_filename, std::ios::out | std::ios::binary);      
   if (!m_fs) error( { "File '", m_filename, "' could not be opened for writing. Check if your user has enough privileges." } );

   // Write the binary and close
   m_fs.write(m_theFile, m_fileSize);
   m_fs.close();
}


///////////////////////////////////////////////////////////////////////////////
// PRINT
//    Pretty prints binary results of patching the file
//
void
Patcher::print(std::ostream& out) const {
   std::string charview;
   charview.reserve(16);

   // Print out header
   out << "File: " << m_filename << ". Size: " << m_fileSize << " bytes.\n";
   out << std::hex << "000000: ";
   
   // Print by rows of 16-bytes wide
   for(std::size_t i=0; i < m_fileSize; ++i) {
      // Print byte and add to charview
      out << std::setw(2) << std::setfill('0') 
          << (+m_theFile[i] & 0xFF) << " ";
      charview.push_back(m_theFile[i]);
      
      // Check 16 bytes already printed out
      if ( charview.size() == 16 ) { 
         // Print last 16 characters and next header
         out   << charview << "\n" 
               << std::setw(6) << i+1 << ": ";
         charview.clear();
      }
   }
   // If there are chars pending to be written, write them out
   if ( charview.size() ) {
      out << std::string(3 * (16 - charview.size()) , ' ')
          << charview;
   }
   // Final end of line
   out << '\n';
}

} // End Namespace CPCT