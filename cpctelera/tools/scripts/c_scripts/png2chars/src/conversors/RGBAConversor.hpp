#pragma once

#include <helpers.h>

#include <cstdint>
#include <ostream>
#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>

struct RGBAConversor {
   using TArrayChar8x8 = std::array<char, 8>;

   // Explicit construction (non-default)
   explicit RGBAConversor(const unsigned char* img, uint64_t w, uint64_t h) 
      : m_image(img), m_width(w), m_height(h) {};
   virtual ~RGBAConversor() {}

   // Noncopyable
   RGBAConversor(const RGBAConversor&)             = delete;
   RGBAConversor& operator=(const RGBAConversor&)  = delete;

   // Public interface
   void convert(std::ostream& out) const  { convert_p(out); }
   void setSymbol(char sym)               { setSymbol_p(sym); }
   void setLine(uint16_t l)               { setLine_p(l); }
   void setCIdentifier(const char* cid)   { setCIdentifier_p(cid); }
   void setLocalConstant(bool l)          { setLocalConstant_p(l); }
   void setCharPrefix(char p)             { setCharPrefix_p(p); }
   void setNumberFormat(TNumberFormat nf) { m_format = nf; }

protected:
   // Pure virtual function to make this class abstract (non-instantiable)
   virtual void convert_p(std::ostream& out) const = 0;

   // Defaulted behaviours (overridable)
   virtual void start_p() const { m_convWidth = m_convHeight = 0; };
   virtual void setSymbol_p(char sym) { }
   virtual void setLine_p(uint16_t l) { }
   virtual void setCIdentifier_p(const char* cid) { }
   virtual void setLocalConstant_p(bool l) { }
   virtual void setCharPrefix_p(char p) { }

   // Protected functions
   TArrayChar8x8 getNextChar_p() const {
      TArrayChar8x8 chardef{};

      // If coordinates outside image, return blank character
      if (m_convWidth >= m_width || m_convHeight >= m_height)
         return chardef;

      // Set pointer to current widht and height
      const unsigned char* pixels = m_image;
      pixels +=   m_convHeight * m_width*4
               +  m_convWidth * 4;

      // Perform conversion
      for (uint16_t i=0; i < 8;  ++i) {
         char row { 0 };
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

   void outputTextHeader_p(std::ostream& out, const char* comment) const {
      using namespace std::chrono;

      uint64_t numchars = m_width*m_height / 64;
      auto now = system_clock::to_time_t( system_clock::now() );
      std::string date = std::ctime( &now );

      out << comment << "\n";
      out << comment << " File generated using CPCtelera/cpct_png2chars\n";
      out << comment << " Date generated: " << date;
      out << comment << " Original image size: " << m_width << "x" << m_height << " pixels\n";
      out << comment << " Characters generated: " << numchars << "\n";
      out << comment << " Output buffer size: " << 8*numchars << " bytes\n";
      out << comment << "\n";
   }

   int16_t cast8bit2print(char n) const {
      return (n + 0) & 0xFF;
   }
   uint16_t cast8bit2print(uint8_t n) const {
      return (n + 0) & 0xFF;
   }
   int16_t cast8bit2print(int8_t n) const {
      return (n + 0) & 0xFF;
   }

   template <typename NumType>
   void print8bitNumberFormatted(std::ostream &out, NumType n8bits, bool basic=false) const {
      auto num = cast8bit2print(n8bits);
      switch (m_format) {
         case TNumberFormat::decimal:     { 
            out << std::dec << std::setw(3) << std::setfill(' ') << num;
            break;
         }
         case TNumberFormat::hexadecimal: { 
            if (basic) out << "&";
            else       out << "0x";
            out << std::hex << std::setw(2) << std::setfill('0') << num; break; 
            break;
         }
         case TNumberFormat::binary_text: { 
            if (basic) out << "&x";
            else       out << "0b";
            for (uint8_t i = 0; i < 8; ++i) {
               if ( num & 0x80 ) out << '1';
               else              out << '0';
               num <<= 1;
            }
            break;
         }
      }
   }

   // Protected members
   TNumberFormat        m_format { TNumberFormat::hexadecimal };
   const unsigned char* m_image {nullptr};
   uint64_t             m_width {0}, m_height {0};
   mutable uint64_t     m_convWidth {0}, m_convHeight {0};
};
