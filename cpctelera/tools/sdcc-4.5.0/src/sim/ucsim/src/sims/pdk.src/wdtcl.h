/*
 * Simulator of microcontrollers (wdtcl.h)
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

#ifndef WDTCL_HEADER
#define WDTCL_HEADER

#include "hwcl.h"

#include "pdkcl.h"


enum wdt_cfg {
  wdt_on	= 0,
  wdt_cnt	= 1,
  wdt_nuof	= 2
};

class cl_wdt: public cl_hw
{
public:
  bool run;
  class cl_pdk *puc;
  class cl_memory_cell *mod, *misc;
  t_mem cnt;
  t_mem len;
  u32_t last;
public:
  cl_wdt(class cl_uc *auc, const char *aname);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return wdt_nuof; }
  virtual const char *cfg_help(t_addr addr);
  virtual void reset(void);
  virtual void recalc(void);
  virtual void set_len(void);
  virtual void clear(void) { cnt= 0; }
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual int tick(int cycles);
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of pdk.src/wdtcl.h */
