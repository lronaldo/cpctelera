/*
 * Simulator of microcontrollers (s51.src/uc88xcl.h)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#ifndef UC88XCL_HEADER
#define UC88XCL_HEADER

#include "uc52cl.h"

class cl_uc88x: public cl_uc52
{
 public:
  cl_uc88x(struct cpu_entry *Itype, class cl_sim *asim);
  virtual int init(void);
  virtual void mk_hw_elements(void);
};


#endif

/* End of s51.src/uc88xcl.h */
