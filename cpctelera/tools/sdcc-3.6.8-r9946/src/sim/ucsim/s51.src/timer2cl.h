/*
 * Simulator of microcontrollers (timer2cl.h)
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

#ifndef TIMER2CL_HEADER
#define TIMER2CL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"

#include "newcmdcl.h"

#include "timer0cl.h"


#define T2MODE_RELOAD	0
#define T2MODE_CAPTURE	1
#define T2MODE_BAUDRATE	2
#define T2MODE_OFF	3
#define T2MODE_DOWN	4
#define T2MODE_CLKOUT	5

enum t2_features {
  t2_default	= 0x01,
  t2_down	= 0x02,
  t2_clock_out	= 0x04
};


class cl_timer2: public cl_timer0
{
protected:
  class cl_address_space *sfr;
  int features;
  class cl_it_src *exf2it;
  t_mem mask_RCLK, mask_TCLK, mask_CP_RL2;
  t_mem RCLK, TCLK, CP_RL2, EXEN2;
  long t2ex_edge;
  class cl_memory_cell *cell_rcap2l, *cell_rcap2h, *cell_t2mod;
  bool bit_dcen, bit_t2oe, bit_t2ex;
public:
  cl_timer2(class cl_uc *auc, int aid, const char *aid_string, int afeautres);
  virtual int init(void);

  virtual void added_to_uc(void);
  //virtual void new_hw_added(class cl_hw *new_hw);

  //virtual ulong read(class cl_mem *mem, long addr);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual void mem_cell_changed(class cl_mem *mem, t_addr addr);

  virtual int  tick(int cycles);
  virtual int  do_t2_baud(int cycles);
  virtual void do_t2_capture(int cycles);
  virtual void do_t2_reload(int cycles);
  virtual void do_t2_down(int cycles);
  virtual void do_t2_clock_out(int cycles);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of s51.src/timer2cl.h */
