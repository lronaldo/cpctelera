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
#include <lib/picopng.hpp>
#include <helpers.h>

#include <vector>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <array>
#include <iomanip>
#include <memory>


PNGDecoder::PNGDecoder() {}

void 
PNGDecoder::readFile(std::string filename) {
   // Save filename for later use
   m_filename = filename;

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

   // Set-up conversors array
   auto cnit = m_conversors.begin();
   cnit->conv = std::make_unique<CConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_C; cnit->ext = ".c"; cnit->formatName = "C-SOURCE"; ++cnit;
   cnit->conv = std::make_unique<ASMConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_S; cnit->ext = ".s"; cnit->formatName = "ASM-ASZ80-SOURCE"; ++cnit;
   cnit->conv = std::make_unique<BASICConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_BAS; cnit->ext = ".bas"; cnit->formatName = "BASIC"; ++cnit;
   cnit->conv = std::make_unique<BINConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_BIN; cnit->ext = ".bin"; cnit->formatName = "RAW BINARY"; ++cnit;
   cnit->conv = std::make_unique<HConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_H; cnit->ext = ".h"; cnit->formatName = "C-HEADER"; ++cnit;
   cnit->conv = std::make_unique<HSConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_HS; cnit->ext = ".h.s"; cnit->formatName = "ASM-ASZ80-HEADER"; ++cnit;
   cnit->conv = std::make_unique<TerminalTestDrawConversor>(m_image.data(), m_width, m_height);
   cnit->flag = GN_DRAW; cnit->ext = ".txt"; cnit->formatName = "TEXT/TERMINAL DRAWING"; ++cnit;
}

void
PNGDecoder::setForAll(std::function<void(ConversorPack&)>&& f) {
   for(auto&& c : m_conversors) { f(c); }
}

void 
PNGDecoder::convert(std::string& outputFolder, std::string& fileBaseName) const {
   if ( outputFolder != "" ) {
      // General message when converting to files
      std::cerr << C_INVERTED_LIGHT_YELLOW << " CPCT_PNG2CHARS: " << C_LIGHT_YELLOW << " Conversion Started ("<< C_YELLOW << fileBaseName << C_LIGHT_YELLOW << ")\n";
      std::cerr << C_LIGHT_CYAN     << "Converting file:     '"<< C_WHITE << m_filename << C_LIGHT_CYAN << "'\n";
      std::cerr << C_LIGHT_CYAN     << "File size:           '"<< C_WHITE << m_width << "x" << m_height << C_LIGHT_CYAN << "' pixels" << "\n";
      std::cerr << C_LIGHT_CYAN     << "Selected conversors: { " << C_WHITE;
      for(auto&& c : m_conversors) {
         if ( m_generate & c.flag ) {
            std::cerr << c.formatName << " ";
         }
      }
      std::cerr << C_LIGHT_CYAN << "}\n";
      std::cerr << C_LIGHT_CYAN     << "Output folder:       '" << C_WHITE << outputFolder << C_LIGHT_CYAN << "'\n";
      std::cerr << C_LIGHT_GREEN    << "Converting...\n";

      // Converting To Files
      for(auto&& c : m_conversors) {
         if ( m_generate & c.flag ) {
            std::string filename = outputFolder + "/" + fileBaseName + c.ext;

            std::cerr << C_LIGHT_CYAN << " * ";
            std::cerr << C_BLUE << "Producing file '"<< C_WHITE << filename << C_BLUE << "'\t (" << C_WHITE << c.formatName << C_BLUE << ") ...";

            std::ofstream file(filename);
            if (!file) error({ "There was an error opening file '", filename, "' for writting. Please check output folder, permissions and disk space." });
            c.conv->convert(file);

            std::cerr << C_LIGHT_GREEN << "Ok!\n";
         }
      }      
      std::cerr << C_INVERTED_LIGHT_GREEN << " CPCT_PNG2CHARS: " << C_LIGHT_GREEN << " Conversion finished ("<< C_GREEN << fileBaseName << C_LIGHT_GREEN << ")\n";
      std::cerr << C_NORMAL;
   } else {
      // Converting to STD::COUT
      for(auto&& c : m_conversors) {
         if ( m_generate & c.flag ) {
            c.conv->convert(std::cout);
         }
      }
   }
}
