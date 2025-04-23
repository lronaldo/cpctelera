/*
 * Simulator of microcontrollers (timercl.h)
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

#ifndef TIMERCL_HEADER
#define TIMERCL_HEADER

#include "hwcl.h"


enum timer_modes {
  tm_stop= 0,
  tm_counter= 1,
  tm_timer= 2
};

enum timer_confs
  {
    tcfg_on	= 0,
    tcfg_mode	= 1,
    tcfg_pre16	= 2,
    tcfg_pre	= 3,
    tcfg_tmr	= 4,
    tcfg_ovflag	= 5,
    tcfg_tflag	= 6,
    tcfg_ien	= 7,
    tcfg_eni	= 7,
    tcfg_nuof	= 8
  };

class cl_timer: public cl_hw
{
 public:
  unsigned int pre, pre16, tmr;
  enum timer_modes mode;
  bool int_enabled, overflow_flag, timer_flag;
 public:
  cl_timer(class cl_uc *auc);
  virtual int init(void);
  virtual void reset(void);
  virtual void print_info(class cl_console_base *con);
  virtual int tick(int cycles);
  virtual void do_timer(unsigned int cyc);
  virtual void do_counter(unsigned int cyc);
  virtual void do_overflow(void);
  virtual unsigned int cfg_size(void) { return tcfg_nuof; }
  virtual const char *cfg_help(t_addr addr);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};


#endif

/* End of i8048.src/timercl.h */
