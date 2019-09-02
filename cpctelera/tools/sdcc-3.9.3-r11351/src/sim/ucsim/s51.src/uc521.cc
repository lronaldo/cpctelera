/*
 * Simulator of microcontrollers (s51.src/uc521.cc)
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

#include "uc521cl.h"


cl_uc521::cl_uc521(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc52(Itype, asim)
{
}

int
cl_uc521::init(void)
{
  int ret;
  ret= cl_uc52::init();

  cpu->cfg_set(uc51cpu_aof_mdps, 0x86);
  cpu->cfg_set(uc51cpu_mask_mdps, 1);
  cpu->cfg_set(uc51cpu_aof_mdps1l, 0x84);
  cpu->cfg_set(uc51cpu_aof_mdps1h, 0x85);
  decode_dptr();

  return ret;
}


/* End of s51.src/uc521.cc */
