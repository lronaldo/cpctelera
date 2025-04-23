/*
 * Simulator of microcontrollers (mos6510cl.h)
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

#ifndef MOS6510CL_HEADER
#define MOS6510CL_HEADER

#include "mos6502cl.h"


class cl_mos6510: public cl_mos6502
{
 public:
  cl_mos6510(class cl_sim *asim);
  virtual int init(void);
  virtual void mk_hw_elements(void);
};

class cl_mos8502: public cl_mos6510
{
 public:
  cl_mos8502(class cl_sim *asim);
  virtual int init(void);
  virtual double def_xtal(void) { return 2000000; }
};


#endif

/* End of mos6502.src/mos6510cl.cc */
