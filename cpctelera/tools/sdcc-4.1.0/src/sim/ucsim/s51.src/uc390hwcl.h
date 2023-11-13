/*
 * Simulator of microcontrollers (serialcl.h)
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

#ifndef UC390HWCL_HEADER
#define UC390HWCL_HEADER

#include "uccl.h"

#include "uc390cl.h"


class cl_uc390_hw: public cl_hw
{
protected:
  class cl_address_space *sfr;
  class cl_memory_cell *cell_dps, *cell_exif, *cell_p4cnt, *cell_acon,
    *cell_p5cnt, *cell_c0c, *cell_pmr, *cell_mcon,
    *cell_ta, *cell_cor, *cell_mcnt0, *cell_mcnt1,
    *cell_ma, *cell_mb, *cell_mc, *cell_wdcon, *cell_c1c;
  class cl_uc390 *uc390;
  unsigned long ctm_ticks; /* mini-state-machine for "crystal multiplier" */
  unsigned long timed_access_ticks;
  int timed_access_state; /* 0: idle; 1: $aa written; 2: $55 written */
public:
  cl_uc390_hw (class cl_uc *auc);
  virtual int init (void);
  //virtual const char *cfg_help(t_addr addr);

  virtual t_mem read (class cl_memory_cell *cell);
  virtual void write (class cl_memory_cell *cell, t_mem *val);

  //virtual void mem_cell_changed (class cl_mem *mem, t_addr addr);

  virtual void reset (void);
  virtual void print_info (class cl_console_base *con);
};


#endif

/* End of s51.src/serialcl.h */
