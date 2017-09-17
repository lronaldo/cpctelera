/*
 * Simulator of microcontrollers (sim.src/vcdcl.h)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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

#ifndef VCDCL_HEADER
#define VCDCL_HEADER

#include "hwcl.h"


class cl_vcd: public cl_hw
{
 protected:
  class cl_list *locs;
  bool started, paused;
  class cl_f *fout;
  bool change;
  double change_time;
  chars modul;
 public:
  cl_vcd(class cl_uc *auc, int aid, chars aid_string);

  virtual void add(class cl_memory_cell *cell);
  virtual bool add(class cl_memory *m, t_addr a, class cl_console_base *con);
  virtual void del(class cl_memory_cell *cell);
  virtual bool del(class cl_memory *m, t_addr a, class cl_console_base *con);
  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void report(class cl_memory_cell *cell, int nr);
  virtual int tick(int cycles);

  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of sim.src/vcdcl.h */
