#pragma once

#include <conversors/RGBAConversor.hpp>
#include <ostream>
#include <cstdint>

struct HSConversor : public RGBAConversor {
   explicit HSConversor(const unsigned char* img, uint64_t w, uint64_t h) 
      : RGBAConversor (img, w, h) {}

   // Inherited convert();
protected:
   void convert_p(std::ostream& out) const override final;
   void setSymbol_p(char sym) override final { m_firstUDG = sym; }
   void setCIdentifier_p(const char* cid) override final { m_identifier = cid; }  
   void setLocalConstant_p(bool l) override final { m_asmLocalConstant = l; }
   void setCharPrefix_p(char p) override final { m_idPrefix = p; }

private:
   char        m_idPrefix     { ' ' };
   bool        m_asmLocalConstant { false };
   char        m_firstUDG     { 32 };
   std::string m_identifier   { "chardata" };
};