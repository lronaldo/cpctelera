/*
 * Simulator of microcontrollers (sim.src/port_hwcl.h)
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

#ifndef PORT_HW_HEADER
#define PORT_HW_HEADER

#include "newcmdposixcl.h"

#include "hwcl.h"


extern const char *keysets[8];


class cl_port_io: public cl_hw_io
{
 public:
  cl_port_io(class cl_hw *ihw);
  virtual int init(void);
  //virtual bool input_avail(void);  
};

class cl_port_data: public cl_base
{
 public:
  class cl_memory_cell *cell_p, *cell_in, *cell_dir;
  t_mem cache_p, cache_in, cache_dir, cache_value;
  const char *keyset;
  int basx, basy, width;
  virtual int init(void) { width=8; return 0; }
};

enum { NUOF_PORT_UIS= 16 };

class cl_port_ui: public cl_hw
{
 public:
  class cl_port_data pd[16];
  int act_port;
 public:
  cl_port_ui(class cl_uc *auc, int aid, chars aid_string);

  virtual bool add_port(class cl_port_data *p, int nr);
  
  virtual void make_io(void);
  virtual void new_io(class cl_f *f_in, class cl_f *f_out);
  virtual bool proc_input(void);
  virtual bool handle_input(int c);
  virtual void refresh_display(bool force);
  virtual void draw_display(void);
};


#endif

/* End of sim.src/port_hwcl.h */
