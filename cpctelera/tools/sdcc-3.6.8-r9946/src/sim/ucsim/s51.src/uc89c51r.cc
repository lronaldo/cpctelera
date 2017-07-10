/*
 * Simulator of microcontrollers (uc89c51r.cc)
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

#include "ddconfig.h"

#include <stdio.h>
#include <ctype.h>

// local
#include "uc89c51rcl.h"
#include "regs51.h"
#include "pcacl.h"
#include "wdtcl.h"


cl_uc89c51r::cl_uc89c51r(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc51r(Itype, asim)
{
}

int
cl_uc89c51r::init(void)
{
  int r= cl_uc51r::init();

  cpu->cfg_set(uc51cpu_aof_mdpc, 0xA2);
  cpu->cfg_set(uc51cpu_mask_mdpc, 1);
  class cl_memory_chip *dptr_chip=
    new cl_memory_chip("dptr_chip", 3*8, 8);
  dptr_chip->init();
  memchips->add(dptr_chip);
  decode_dptr();
  
  return r;
}

void
cl_uc89c51r::mk_hw_elements(void)
{
  class cl_hw *h;

  cl_uc52::mk_hw_elements();
  add_hw(h= new cl_wdt(this, 0x3fff));
  h->init();
  add_hw(h= new cl_pca(this, 0));
  h->init();
}

void
cl_uc89c51r::make_memories(void)
{
  cl_uc52::make_memories();
}

void
cl_uc89c51r::reset(void)
{
  cl_uc51r::reset();
  sfr->set_bit1(CCAPM0, bmECOM);
  sfr->set_bit1(CCAPM1, bmECOM);
  sfr->set_bit1(CCAPM2, bmECOM);
  sfr->set_bit1(CCAPM3, bmECOM);
  sfr->set_bit1(CCAPM4, bmECOM);
  sfr->write(IPH, 0);
}

int
cl_uc89c51r::it_priority(uchar ie_mask)
{
  uchar l, h;

  l= sfr->get(IP) & ie_mask;
  h= sfr->get(IPH) & ie_mask;
  if (!h && !l)
    return(0);
  if (!h && l)
    return(1);
  if (h && !l)
    return(2);
  if (h && l)
    return(3);
  return(0);
}


/* End of s51.src/uc89c51r.cc */
