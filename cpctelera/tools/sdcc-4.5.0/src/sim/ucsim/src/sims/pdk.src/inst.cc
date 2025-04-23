/*
 * Simulator of microcontrollers (inst.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include <stdlib.h>
#include <cassert>
#include "ddconfig.h"
#include "stdio.h"

// local
#include "pdkcl.h"
#include "regspdk.h"


u8_t cl_fpp::add_to(u8_t initial, int value, bool carry) {
  u8_t f= 0, res;
  int c= carry?(fC):0;
  int r, r2, r3;

  r= (int)initial + (int)value + c;
  r2= (initial & 0x7f) + (value & 0x7f) + c;
  r3= (initial &  0xf) + (value &  0xf) + c;
  if ((r & 0xff) == 0) f|= BIT_Z;
  if (r > 0xff) f|= BIT_C;
  if (r3 > 0xf) f|= BIT_AC;
  if ( (r2 & 0x80) && !(f & BIT_C)) f|= BIT_OV;
  if (!(r2 & 0x80) &&  (f & BIT_C)) f|= BIT_OV;
  cF->W(f);
  return r;
  
  store_flag(flag_z, initial + value + carry == 0);
  store_flag(flag_c, initial + value + carry > 0xFF);
  store_flag(flag_ac, (initial & 0xF) + (value & 0xF) + carry > 0xF);
  store_flag(
      flag_ov,
      fC ^ ((initial & 0x7F) + (value & 0x7F) + carry > 0x7F));
  return initial + value + carry;
}

u8_t cl_fpp::sub_to(u8_t initial, int value, bool carry) {
  u8_t f= 0;
  int r= initial - value - carry;
  
  if ((r&0xff) == 0) f|= BIT_Z;
  if (initial < value + carry) f|= BIT_C;
  if ((value & 0xF) > (initial & 0xF) - carry) f|= BIT_AC;
  if (((f&BIT_C)>>BITPOS_C) ^ ((initial & 0x7F) - (value & 0x7F) - carry < 0))
    f|= BIT_OV;

  cF->W(f);
  return r;
}
/*
int cl_fppa::store_io(t_addr addr, int value) {
  
  sfr->write(addr, value & 0xFF);
  if (addr == 0x02)
    {
      if (rSP > sp_most)
        sp_most = rSP;
      if (!ram->valid_address(value))
        return resSTACK_OV;
    }
  return resGO;
}
*/
void cl_fpp::store_flag(flag n, int value) {
  value= value?1:0;
  switch (n) {
  case flag_z: cF->W((rF & ~1) | (value << BITPOS_Z)); break;
  case flag_c: cF->W((rF & ~2) | (value << BITPOS_C)); break;
  case flag_ac: cF->W((rF & ~4) | (value << BITPOS_AC)); break;
  case flag_ov: cF->W((rF & ~8) | (value << BITPOS_OV)); break;
  default:
    assert(!"invalid bit store to FLAG");
  }
}


/* End of pdk.src/inst.cc */
