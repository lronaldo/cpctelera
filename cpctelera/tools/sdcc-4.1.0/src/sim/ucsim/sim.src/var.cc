/*
 * Simulator of microcontrollers (sim.src/var.cc)
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

#include <string.h>
#include <ctype.h>

#include "varcl.h"


cl_var::cl_var(const char *iname, class cl_address_space *ias, t_addr iaddr, chars adesc, int ibitnr):
  cl_base()
{
  as= ias;
  addr= iaddr;
  bitnr= ibitnr;
  desc= adesc;
  
  set_name(iname);
  
  cell= NULL;
}

int
cl_var::init(void)
{
  if (!as ||
      !as->is_address_space() ||
      !as->valid_address(addr))
    return 0;
  cell= as->get_cell(addr);
  if (cell && (bitnr < 0))
    cell->set_flag(CELL_VAR, true);
  return 0;
}

int
cl_var::move(t_addr new_addr)
{
  if (addr == new_addr)
    return 0;
  if (cell && (bitnr < 0))
    cell->set_flag(CELL_VAR, false);
  addr= new_addr;
  cell= NULL;
  return init();
}

t_mem
cl_var::write(t_mem val)
{
  if (!cell)
    return 0;
  return cell->write(val);
}

t_mem
cl_var::set(t_mem val)
{
  if (!cell)
    return 0;
  return cell->set(val);
}

void
cl_var::print_info(cl_console_base *con)
{
  con->dd_printf("%s ", get_name("?"));
  if (cell)
    {
      t_mem v= cell->read();
      con->dd_printf("%s", as->get_name("?"));
      con->dd_printf("[");
      con->dd_printf(as->addr_format, addr);
      con->dd_printf("] ");
      if (bitnr >= 0)
	{
	  con->dd_printf(".%d", bitnr);
	  con->dd_printf("= %d", (v & (1<<bitnr))?1:0);
	}
      else
	{
	  con->dd_printf("= ");
	  con->dd_printf(as->data_format, v);
	  con->dd_printf(",%u", MU(v));
	  con->dd_printf(",%d", MI(v));
	  if (isprint(MI(v)))
	    con->dd_printf(",'%c'", MI(v));
	}
    }
  con->dd_printf("\n");
  if (!desc.empty())
    con->dd_printf("  %s\n", desc.c_str());
}


const void *
cl_var_list::key_of(const void *item) const
{
  return ((const class cl_var *)item)->get_name();
}

int
cl_var_list::compare(const void *key1, const void *key2)
{
  const char *k1, *k2;

  k1= (const char*)key1;
  k2= (const char*)key2;
  if (k1 && k2)
    return strcmp(k1, k2);
  return 0;
}


/* End of sim.src/var.cc */
