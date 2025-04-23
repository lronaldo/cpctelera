/*
 * Simulator of microcontrollers (timercl.h)
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

#ifndef TIMERCL_HEADER
#define TIMERCL_HEADER

#include "hwcl.h"


class cl_timer: public cl_hw
{
 public:
  class cl_memory_cell *ctrl, *ar, *cnt, *stat;
  t_addr addr;
  u32_t cps, pre_cnt;
  u32_t overflow;
 public:
  cl_timer(class cl_uc *auc, t_addr the_addr, const char *aname);
  virtual int init(void);
  virtual void reset(void);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual int tick(int cycles);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of p1516.src/timercl.h */
