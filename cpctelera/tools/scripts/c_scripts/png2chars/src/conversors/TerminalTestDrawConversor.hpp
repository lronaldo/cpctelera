#pragma once

#include <conversors/RGBAConversor.hpp>
#include <ostream>
#include <cstdint>

struct TerminalTestDrawConversor : public RGBAConversor {
   explicit TerminalTestDrawConversor(const unsigned char* img, uint64_t w, uint64_t h) 
      : RGBAConversor (img, w, h) {}

   // Inherited convert();
protected:
   void drawChar8x8(std::ostream& out, const TArrayChar8x8& chardef) const;
   void convert_p(std::ostream& out) const override final;
};