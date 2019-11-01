#pragma once

#include <conversors/RGBAConversor.hpp>
#include <ostream>
#include <cstdint>

struct HConversor : public RGBAConversor {
   explicit HConversor(const unsigned char* img, uint64_t w, uint64_t h) 
      : RGBAConversor (img, w, h) {}

   // Inherited convert();
protected:
   void convert_p(std::ostream& out) const override final;
   void setSymbol_p(char sym) override final { m_firstUDG = sym; }
   void setCIdentifier_p(const char* cid) override final { m_c_identifier = cid; }  
private:
   char        m_firstUDG     { 32 };
   std::string m_c_identifier { "chardata" };
};