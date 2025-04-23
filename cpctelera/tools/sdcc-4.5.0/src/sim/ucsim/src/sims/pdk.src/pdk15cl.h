/*
 * Simulator of microcontrollers (pdk15cl.h)
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

#ifndef PDK15CL_HEADER
#define PDK15CL_HEADER

#include "pdk14cl.h"


class cl_fpp15: public cl_fpp14
{
 public:
  cl_fpp15(int aid, class cl_pdk *the_puc, class cl_sim *asim);
  cl_fpp15(int aid, class cl_pdk *the_puc, struct cpu_entry *IType, class cl_sim *asim);
  virtual const char *id_string(void) { return "pdk15"; }
  virtual int m_mask(void) { return 0xff; }
  virtual int io_mask(void) { return 0x7f; }
  virtual int rom_mask(void) { return 0x1fff; }
  virtual struct dis_entry *dis_tbl(void);
  virtual int execute(unsigned int code);
};


#endif

/* End of pdk.src/pdk15cl.h */
