/*
 * Simulator of microcontrollers (wdtcl.h)
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

#ifndef S51_WDTCL_HEADER
#define S51_WDTCL_HEADER

// sim.src
//#include "stypes.h"
//#include "pobjcl.h"
#include "uccl.h"

// local
//#include "newcmdcl.h"
#include "uc51rcl.h"


class cl_wdt: public cl_hw
{
protected:
  long wdt, reset_value;
  class cl_memory_cell *wdtrst;
  bool written_since_reset;
public:
  cl_wdt(class cl_uc *auc, long resetvalue);
  virtual int init(void);
  //virtual const char *cfg_help(t_addr addr);

  //virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual t_mem set_cmd(t_mem value);

  virtual int tick(int cycles);
  virtual void reset(void);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of s51.src/wdtcl.h */
