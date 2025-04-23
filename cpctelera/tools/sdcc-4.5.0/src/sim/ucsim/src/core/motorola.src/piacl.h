/*
 * Simulator of microcontrollers (piacl.h)
 *
 * Copyright (C) 1999 Drotos Daniel
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

#ifndef PIACL_HEADER
#define PIACL_HEADER

#include "hwcl.h"
#include "port_hwcl.h"


enum pia_cfg
  {
   cfg_on	= 0,
   cfg_base	= 1,
   cfg_reqs	= 2,
   cfg_ca1_req	= 3,
   cfg_ca2_req	= 4,
   cfg_cb1_req	= 5,
   cfg_cb2_req	= 6,
   cfg_ddra	= 7,
   cfg_ora	= 8,
   cfg_ina	= 9,
   cfg_ira	= 10,
   cfg_ddrb	= 11,
   cfg_orb	= 12,
   cfg_inb	= 13,
   cfg_irb	= 14,
   cfg_oca	= 15, // out value of CA2
   cfg_ddca	= 16, // data direction of CA2(,CA1)
   cfg_inca	= 17, // input value for CA2,CA1
   cfg_ocb	= 18, // out value of CB2
   cfg_ddcb	= 19, // data direction of CB2(,CB1)
   cfg_incb	= 20, // input value for CB2,CB1
  };

class cl_pia: public cl_hw
{
public:
  t_addr base;
  class cl_memory_cell *cra;
  class cl_memory_cell *ddra;
  class cl_memory_cell *ora;
  class cl_memory_cell *ina;
  class cl_memory_cell *crb;
  class cl_memory_cell *ddrb;
  class cl_memory_cell *orb;
  class cl_memory_cell *inb;
  class cl_memory_cell *oca;
  class cl_memory_cell *ddca;
  class cl_memory_cell *inca;
  class cl_memory_cell *ocb;
  class cl_memory_cell *ddcb;
  class cl_memory_cell *incb;
  class cl_memory_cell *rs[4];
  int prev_ca1, prev_ca2, prev_cb1, prev_cb2;
  class cl_it_src *is_ca1, *is_ca2, *is_cb1, *is_cb2;
 public:
  cl_pia(class cl_uc *auc, int aid);
  cl_pia(class cl_uc *auc, int aid, t_addr the_addr);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return 21; }
  virtual const char *cfg_help(t_addr addr);
  virtual bool set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void set_help(class cl_console_base *con);

  virtual class cl_memory_cell *reg(class cl_memory_cell *cell_rs);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual int ira(void);
  virtual int irb(void);
  virtual int ca1(void);
  virtual int ca2(void);
  virtual int cb1(void);
  virtual int cb2(void);
  
  virtual int check_edges(void);
  virtual int tick(int cycles);
  virtual void reset(void);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of motorola.src/piacl.h */
