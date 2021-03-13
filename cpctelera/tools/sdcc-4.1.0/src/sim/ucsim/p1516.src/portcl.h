/*
 * Simulator of microcontrollers (portcl.h)
 *
 * Copyright (C) 2020,20 Drotos Daniel, Talker Bt.
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

#ifndef PORTCL_HEADER
#define PORTCL_HEADER

#include "port_hwcl.h"


enum port_cfg
  {
   port_pin= 0 // RW
  };


class cl_porto: public cl_hw
{
 public:
  class cl_memory_cell *dr;
  t_addr addr;
  u32_t value, cache;
 public:
  cl_porto(class cl_uc *auc, t_addr the_addr, const char *aname);
  virtual int init(void);
  virtual void reset(void);
  virtual int cfg_size(void) { return 1; }
  //virtual const char *cfg_help(t_addr addr);

  virtual void write(class cl_memory_cell *cell, t_mem *val);
  //virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void print(class cl_console_base *con);
  virtual void print_info(class cl_console_base *con);
  virtual void refresh_display(bool force);
  virtual void draw_display(void);
};

class cl_porti: public cl_porto
{
public:
  cl_porti(class cl_uc *auc, t_addr the_addr, const char *aname);
public:
  virtual int init(void);
  virtual int cfg_size(void) { return 1; }
  virtual const char *cfg_help(t_addr addr);

  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};


#endif

/* End of p1516.src/portcl.h */
