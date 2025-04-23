/*
 * Simulator of microcontrollers (osccl.h)
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

#ifndef OSCCL_HEADER
#define OSCCL_HEADER

#include "hwcl.h"


enum osc_cfg {
  osc_on		= 0,
  osc_freq_ihrc		= 1,
  osc_freq_ilrc		= 2,
  osc_freq_eosc		= 3,
  osc_nuof		= 4
};


class cl_osc: public cl_hw
{
 public:
  class cl_pdk *puc;
  double frsys;
  t_mem frh, frl, fre; // frequencies
  double ihrc, ilrc, eosc, sys; // number of clock periods
  double mh, ml, me; // multiplication factors
  int runh, runl, rune;
  class cl_memory_cell *eoscr, *clkmd;
  chars sys_source;
 public:
  cl_osc(class cl_uc *auc, const char *aname);
  virtual int init(void);
  virtual void recalc(void);
  virtual unsigned int cfg_size(void) { return osc_nuof; }
  virtual const char *cfg_help(t_addr addr);
  virtual void reset(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual int tick(int cycles);
  virtual void setup(t_mem src_fr, unsigned int div_by);
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of pdk.src/osccl.h */
