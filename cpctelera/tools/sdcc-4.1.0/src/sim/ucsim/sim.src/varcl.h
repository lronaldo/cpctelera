/*
 * Simulator of microcontrollers (sim.src/varcl.h)
 *
 * Copyright (C) @@S@@,@@Y@@ Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/*
  This file is part of microcontroller simulator: ucsim.

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
  02111-1307, USA.
*/
/*@1@*/

#ifndef SIM_VARCL_HEADER
#define SIM_VARCL_HEADER


#include "pobjcl.h"

#include "newcmdcl.h"

#include "memcl.h"


class cl_var: public cl_base
{
 public:
  class cl_address_space *as; // reference
  t_addr addr;
  int bitnr;
  chars desc;
 protected:
  class cl_memory_cell *cell;
 public:
  cl_var(const char *iname, class cl_address_space *ias, t_addr iaddr, chars adesc, int ibitnr= -1);
  virtual int init(void);
  virtual int move(t_addr new_addr);
  virtual class cl_memory_cell *get_cell(void) { return cell; }

  virtual t_mem write(t_mem val);
  virtual t_mem set(t_mem val);
  
  virtual void print_info(cl_console_base *con);
};


class cl_var_list: public cl_sorted_list
{
 public:
 cl_var_list(): cl_sorted_list(10, 10, "symlist") {}
 public:
  virtual const void *key_of(const void *item) const;
  virtual int compare(const void *key1, const void *key2);
};


#endif

/* End of sim.src/varcl.h */
