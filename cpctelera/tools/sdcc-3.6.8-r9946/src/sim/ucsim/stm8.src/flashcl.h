/*
 * Simulator of microcontrollers (stm8.src/flashcl.h)
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

#ifndef FLASHCL_HEADER
#define FLASHCL_HEADER

#include "hwcl.h"


enum stm8_flash_cfg {
  stm8_flash_on= 0,
  stm8_flash_nuof_cfg= 1
};

enum stm8_mass {
  PMASS1= 0x56,
  PMASS2= 0xae,
  DMASS1= 0xae,
  DMASS2= 0x56
};

class cl_flash: public cl_hw
{
 protected:
  t_addr base;
  cl_memory_cell *pukr, *dukr, *iapsr;
  bool puk1st, duk1st;
  bool p_unlocked, d_unlocked;
  bool p_failed, d_failed;
 public:
  cl_flash(class cl_uc *auc, t_addr abase, const char *aname);
  virtual int init(void);
  virtual void registration(void) {}
  virtual void reset(void);

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  
  virtual void print_info(class cl_console_base *con);
};

class cl_saf_flash: public cl_flash
{
 public:
  cl_saf_flash(class cl_uc *auc, t_addr abase);
  virtual void registration(void);
};

class cl_l_flash: public cl_flash
{
 public:
  cl_l_flash(class cl_uc *auc, t_addr abase);
  virtual void registration(void);
};

#endif

/* End of stm8.src/flashcl.h */
