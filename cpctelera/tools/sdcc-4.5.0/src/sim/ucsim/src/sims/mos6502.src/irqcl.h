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

#ifndef IRQCL_HEADER
#define IRQCL_HEADER

#include "hwcl.h"

#include "mos6502cl.h"

enum irq_cfg
  {
    m65_nmi_en	= 0,
    m65_nmi	= 1,
    m65_irq_en	= 2,
    m65_irq	= 3,
    m65_brk_en	= 4,
    m65_brk	= 5,
    m65_nr	= 6
  };

// IRQ peripheral
class cl_irq_hw: public cl_hw
{
 public:
  class cl_mos6502 *muc;
 public:
  cl_irq_hw(class cl_uc *auc);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return m65_nr; }
  virtual void print_info(class cl_console_base *con);  
};


#endif

/* End of mos6502.src/irqcl.h */
