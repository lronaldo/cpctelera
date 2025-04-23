/*
 * Simulator of microcontrollers (irqcl.h)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#ifndef IRQ_HEADER
#define IRQ_HEADER

#include "hwcl.h"
#include "itsrccl.h"


enum cpu_cfg
  {
   cpu_nmi_en	= 0,
   cpu_nmi	= 1,
   cpu_irq_en	= 2,
   cpu_irq	= 3,
   cpu_firq_en	= 4,
   cpu_firq	= 5,
   cpu_nr	= 6
  };

enum {
  FIRQ_AT	= 0xfff6,
  IRQ_AT	= 0xfff8,
  NMI_AT	= 0xfffc
};


// Interrupt peripheral
class cl_m6809_irq: public cl_hw
{
public:
  class cl_m6809 *muc;
public:
  cl_m6809_irq(class cl_uc *auc);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return cpu_nr; }
  virtual const char *cfg_help(t_addr addr);
  virtual void reset(void);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual void print_info(class cl_console_base *con);  
};


#endif

/* End of m6809.src/irqcl.h */
