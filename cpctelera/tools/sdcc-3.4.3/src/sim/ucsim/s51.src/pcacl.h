/*
 * Simulator of microcontrollers (pcacl.h)
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

// sim.src
//#include "stypes.h"
//#include "pobjcl.h"
#include "uccl.h"

// local
//#include "newcmdcl.h"


class cl_pca: public cl_hw
{
public:
  class cl_address_space *sfr;
  class cl_memory_cell *cell_cl, *cell_ch;
  class cl_memory_cell *cell_ccapl[5], *cell_ccaph[5], *cell_ccapm[5];
  t_mem ccapm[5];
  class cl_memory_cell *cell_cmod, *cell_ccon;
  long t0_overflows, ECI_edge;
  t_mem clk_source;
  bool bit_CIDL, bit_WDTE, bit_ECF, bit_CR;
  bool cex_pos[5], cex_neg[5];
public:
  cl_pca(class cl_uc *auc, int aid);
  virtual int init(void);

  virtual void added_to_uc(void);

  //virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual t_mem set_cmd(t_mem value);
  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);
 
  virtual int tick(int cycles);
  virtual void do_pca_counter(int cycles);
  virtual void do_pca_module(int nr);
  virtual void reset(void);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);
 
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of s51.src/pcacl.h */
