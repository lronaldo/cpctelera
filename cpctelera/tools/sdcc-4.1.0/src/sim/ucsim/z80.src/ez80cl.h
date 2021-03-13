/*
 * Simulator of microcontrollers (ez80cl.h)
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

#ifndef EZ80CL_HEADER
#define EZ80CL_HEADER

#include "z80cl.h"

class cl_ez80: public cl_z80
{
 public:
 public:
  cl_ez80(struct cpu_entry *Itype, class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual const char *get_disasm_info(t_addr addr,
				      int *ret_len,
				      int *ret_branch,
				      int *immed_offset,
				      struct dis_entry **dentry);
  
  virtual int inst_ed_ez80(t_mem code);    
  virtual int inst_ed(t_mem prefix);
  virtual int inst_dd_spec(t_mem code);
  virtual int inst_fd_spec(t_mem code);
};

#endif

/* End of z80.src/ez80cl.h */
