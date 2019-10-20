#pragma once

#include <cstdint>
#include <ostream>
#include <array>

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
      TArrayChar8x8 chardef;

      // If coordinates outside image, return blank character
      if (m_convWidth >= m_width || m_convHeight >= m_height)
         return chardef;

      // Set pointer to current widht and height
      const unsigned char* pixels = m_image;
      pixels +=  m_convHeight * m_width*4
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

   // Protected members
   const unsigned char* m_image {nullptr};
   uint64_t             m_width {0}, m_height {0};
   mutable uint64_t     m_convWidth {0}, m_convHeight {0};
};