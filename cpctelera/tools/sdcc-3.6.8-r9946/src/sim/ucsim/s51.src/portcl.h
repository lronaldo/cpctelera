/*
 * Simulator of microcontrollers (portcl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"

#include "newcmdcl.h"

#include "port_hwcl.h"


enum port_cfg {
  port_on		= 0, // RW
  port_pin		= 1, // RW
  port_value		= 2, // RO
};

class cl_port: public cl_hw
{
public:
  t_addr addr_p;
  t_mem port_pins;
  t_mem prev;
  class cl_address_space *bas;
  class cl_memory_cell *cell_p, *cell_in, *bit_cells[8];
public:
  cl_port(class cl_uc *auc, int aid);
  cl_port(class cl_uc *auc, int aid, t_addr the_addr);
  virtual int init(void);
  virtual int cfg_size(void) { return 3; }

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void set_pin(t_mem val);
    
  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

  virtual void make_io(void) {}
  //virtual int tick(int cycles);
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of s51.src/portcl.h */
