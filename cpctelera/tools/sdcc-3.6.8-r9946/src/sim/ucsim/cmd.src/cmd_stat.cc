/*
 * Simulator of microcontrollers (cmd.src/cmdstat.cc)
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

// prj
#include "globals.h"

// sim
#include "simcl.h"

// local
#include "cmd_statcl.h"


#ifdef STATISTIC
/*
 * Command: statistic
 *----------------------------------------------------------------------------
 */

//int
//cl_stat_cmd::do_work(class cl_sim *sim,
//		       class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_statistic_cmd)
{
  class cl_address_space *mem;
  t_addr start= 0, end= 0;
  bool addresses= false;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };

  mem= 0;
  if (cmdline->syntax_match(uc, MEMORY ADDRESS ADDRESS)) {
    mem= params[0]->value.memory.address_space;
    start= params[1]->value.address;
    end= params[2]->value.address;
    addresses= true;
  }
  else if (cmdline->syntax_match(uc, MEMORY ADDRESS)) {
    mem= params[0]->value.memory.address_space;
    start= end= params[1]->value.address;
    addresses= true;
  }
  else if (cmdline->syntax_match(uc, MEMORY)) {
    mem= params[0]->value.memory.address_space;
    addresses= false;
  }
  else
    {
      /*con->dd_printf("Error: wrong syntax\n"
	"%s\n", short_help?short_help:"no help");*/
      int i;
      unsigned long wr, ww;
      for (i= 0; i < uc->address_spaces->count; i++)
	{
	  mem= (class cl_address_space *)(uc->address_spaces->at(i));
	  wr= mem->get_nuof_reads();
	  ww= mem->get_nuof_writes();
	  con->dd_printf("%s writes= %10lu "
			 "reads= %10lu "
			 "(%10lu operations)\n",
			 mem->get_name("mem"), ww, wr, ww+wr);
	}
    }
  if (mem)
    {
      t_addr i;
      unsigned long wr, ww;
      wr= mem->get_nuof_reads();
      ww= mem->get_nuof_writes();
      if (!addresses)
	con->dd_printf("%s writes= %10lu "
		       "reads= %10lu\n", mem->get_name("mem"), ww, wr);
      else
	for (i= start; i <= end; i++)
	  {
	    class cl_memory_cell *c= mem->get_cell(i);
	    unsigned long w= c->nuof_writes, r= c->nuof_reads;
	    double dr= wr?((double(r)*100.0)/double(wr)):0.0;
	    double dw= ww?((double(w)*100.0)/double(ww)):0.0;
	    con->dd_printf("%s[0x%06x] writes= %10lu (%6.2lf%%) "
			   "reads= %10lu (%6.2lf%%)\n",
			   mem->get_name("mem"), i, w, dw, r, dr);
	  }
    }

  return(false);;
}
#endif


/* End of cmd.src/cmd_stat.cc */
