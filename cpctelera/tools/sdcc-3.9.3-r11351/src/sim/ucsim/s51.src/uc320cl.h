/*
 * Simulator of microcontrollers (s51.src/uc320cl.h)
 *
 * Copyright (C) 2018,18 whitequark
 *
 * To contact author send email to whitequark@whitequark.org
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

#ifndef UC320CL_HEADER
#define UC320CL_HEADER

#include "uc521cl.h"

class cl_uc320: public cl_uc521
{
 public:
  cl_uc320(struct cpu_entry *Itype, class cl_sim *asim);
  void clear_sfr(void);

  virtual int clock_per_cycle(void) { return(4); }
  virtual int tick(int cycles);
  virtual int tick_hw(int cycles);

 protected:
  int pending_ticks;

  virtual int exec_inst(void);

  virtual int instruction_e0/*inst_movx_a_Sdptr*/(t_mem/*uchar*/ code); /* e0 */
  virtual int instruction_e2/*inst_movx_a_Sri*/(t_mem/*uchar*/ code); /* e2,e3 */
  virtual int instruction_f0/*inst_movx_Sdptr_a*/(t_mem/*uchar*/ code); /* f0 */
  virtual int instruction_f2/*inst_movx_Sri_a*/(t_mem/*uchar*/ code); /* f2,f3 */
};

#endif

/* End of s51.src/uc380cl.h */
