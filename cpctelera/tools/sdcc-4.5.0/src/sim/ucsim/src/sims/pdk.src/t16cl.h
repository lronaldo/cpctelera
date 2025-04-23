/*
 * Simulator of microcontrollers (t16cl.h)
 *
 * Copyright (C) 2024 Drotos Daniel
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

#ifndef T16CL_HEADER
#define T16CL_HEADER

#include "hwcl.h"

#include "pdkcl.h"


enum t16_cfg {
  t16_on	= 0,
  t16_cnt	= 1,
  t16_nuof	= 2
};

class cl_t16: public cl_hw
{
public:
  class cl_pdk *puc;
  class cl_memory_cell *mod, *edg, *irq;
  u16_t cnt, mask;
  double *src;
  int div;
  unsigned int pre;
  u32_t last;
  chars clk_source;
public:
  cl_t16(class cl_uc *auc, const char *aname);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return t16_nuof; }
  virtual const char *cfg_help(t_addr addr);
  virtual void reset(void);
  virtual void recalc(void);
  virtual void set_div(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual int tick(int cycles);
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of pdk.src/t16cl.h */
