#pragma once

#include <conversors/RGBAConversor.hpp>
#include <ostream>
#include <cstdint>

struct ASMConversor : public RGBAConversor {
   explicit ASMConversor(const unsigned char* img, uint64_t w, uint64_t h) 
      : RGBAConversor (img, w, h) {}

   // Inherited convert();
protected:
   void outputCharDefs_p(
         std::ostream& out, const TArrayChar8x8& chardef
      ,  uint16_t charnum) const;
 
   void convert_p(std::ostream& out) const override final;
   void setSymbol_p(char sym) override final { m_firstUDG = sym; }
   void setCIdentifier_p(const char* cid) override final { m_identifier = cid; }  
   void setLocalConstant_p(bool l) override final { m_asmLocalConstant = l; }
   void setCharPrefix_p(char p) override final { m_idPrefix = p; }

private:
   bool        m_asmLocalConstant { false };
   char        m_idPrefix { ' ' };
   char        m_firstUDG { 32 };
   std::string m_identifier { "chardata" };
};