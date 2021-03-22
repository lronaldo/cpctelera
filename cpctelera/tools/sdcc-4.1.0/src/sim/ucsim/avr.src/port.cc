/*
 * Simulator of microcontrollers (port.cc)
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


//#include <stdio.h>

#include "portcl.h"


cl_port::cl_port(class cl_uc *auc):
  cl_hw(auc, HW_PORT, 0, "port")
{
  //uc->register_hw_read(MEM_SFR, 2, this);
  //uc->register_hw_read(MEM_SFR, 4, this);
}

/*ulong
cl_port::read(class cl_mem *mem, long addr)
{
  switch (addr)
    {
    case 2:
      return(22);
    case 4:
      return(44);
    }
  return(cl_hw::read(mem, addr));
}*/


/* End of avr.src/port.cc */
