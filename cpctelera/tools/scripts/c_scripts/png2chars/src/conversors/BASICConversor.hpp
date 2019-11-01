#pragma once

#include <conversors/RGBAConversor.hpp>
#include <ostream>
#include <cstdint>

struct BASICConversor : public RGBAConversor {
   explicit BASICConversor(const unsigned char* img, uint64_t w, uint64_t h) 
      : RGBAConversor (img, w, h) {}

   // Inherited convert();
protected:
   void outputCharDefs_p(
         std::ostream& out, const TArrayChar8x8& chardef
      ,  uint16_t symbol, uint16_t line) const;
 
   void convert_p(std::ostream& out) const override final;
   void setSymbol_p(char sym) override final { m_firstUDG = sym; }
   void setLine_p(uint16_t l) override final { m_firstLine = l; }

private:
   char     m_firstUDG  { 32 };
   uint16_t m_firstLine { 10 };
};