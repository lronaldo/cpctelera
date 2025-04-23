/*
 * Simulator of microcontrollers (i8041cl.h)
 *
 * Copyright (C) 2022 Drotos Daniel
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


#ifndef I8041CL_HEADER
#define I8041CL_HEADER

#include "i8048cl.h"


class cl_i8041: public cl_i8048
{
 public:
  cl_i8041(class cl_sim *asim);
  cl_i8041(class cl_sim *asim,
	   unsigned int rom_siz,
	   unsigned int ram_siz);
  virtual int init(void);
  virtual class cl_memory_operator *make_flagop(void);
  virtual void decode_regs(void);

  // UPI41 sepcific instructions to implement
  //OUTDBB
  //INDBB
};


class cl_i8041A: public cl_i8041
{
 public:
  cl_i8041A(class cl_sim *asim);
  cl_i8041A(class cl_sim *asim,
	   unsigned int rom_siz,
	   unsigned int ram_siz);
  virtual int init(void);
};


#endif

/* End of i8048.src/i8041cl.h */
