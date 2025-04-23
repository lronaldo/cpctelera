/*
 * Simulator of microcontrollers (mos65ce02cl.h)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#ifndef MOS65CE02CL_HEADER
#define MOS65CE02CL_HEADER

#include "mos65c02scl.h"

#define rB (B)
#define rZ (Z)


class cl_mos65ce02: public cl_mos65c02s
{
public:
  u8_t B, Z;
  class cl_cell8 cB, cZ;
public:
  cl_mos65ce02(class cl_sim *asim);
  virtual int init(void);

  virtual void print_regs(class cl_console_base *con);
};


#endif

/* End of mos6502.src/mos65ce02.cc */
