/*
 * Simulator of microcontrollers (port10cl.h)
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

#ifndef PORT10CL_HEADER
#define PORT10CL_HEADER

#include "hwcl.h"


enum port10_cfg
  {
    port10_on	= 0, // RW
    port10_pin	= 1, // RW
    port10_port	= 2, // RO
    port10_nuof	= 3
  };

class cl_port10: public cl_hw
{
public:
  class cl_cell8 *cdr; // Data reg, address= 1
  class cl_cell8 *cddr; // Data direction reg, address= 0
  class cl_cell8 *cpin; // Pins in cfg
public:
  cl_port10(class cl_uc *auc, const char *aname);
  virtual unsigned int cfg_size(void) { return port10_nuof; }
  virtual int init(void);
  virtual void reset(void);
  virtual u8_t val(void);
  
  virtual t_mem read(class cl_memory_cell *cell);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual const char *cfg_help(t_addr addr);

  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of mos6502.src/port10cl.h */
