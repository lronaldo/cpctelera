/*
 * Simulator of microcontrollers (timer0cl.h)
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

#ifndef TIMER0CL_HEADER
#define TIMER0CL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"

#include "newcmdcl.h"

#include "uc51cl.h"


class cl_timer0: public cl_hw
{
protected:
  class cl_memory_cell *cell_tmod, *cell_tcon, *cell_tl, *cell_th;
  class cl_memory_cell *tcon_bits[8];
  class cl_address_space *bas;
  t_mem mask_M0, mask_M1, mask_C_T, mask_GATE, mask_TR, mask_INT,
    mask_T, mask_TF;
  t_addr addr_tl, addr_th, addr_tcon;
  int mode, GATE, C_T, TR, INT, T_edge;
public:
  cl_timer0(class cl_uc *auc, int aid, const char *aid_string);
  virtual int init(void);
  //virtual const char *cfg_help(t_addr addr);
  
  virtual void added_to_uc(void);

  //virtual t_mem read(class cl_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  
  //virtual void mem_cell_changed(class cl_mem *mem, t_addr addr);

  virtual int tick(int cycles);
  virtual int do_mode0(int cycles);
  virtual int do_mode1(int cycles);
  virtual int do_mode2(int cycles);
  virtual int do_mode3(int cycles);
  virtual void overflow(void);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);

  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of s51.src/timer0cl.h */
