/*
 * Simulator of microcontrollers (cmd.src/cmdgui.cc)
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
#include "cmd_guicl.h"


/*
 * Command: gui start
 *----------------------------------------------------------------------------
 */

//int
//cl_gui_start_cmd::do_work(class cl_sim *sim,
//			  class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_gui_start_cmd)
{
  if (cmdline->syntax_match(uc, MEMORY ADDRESS ADDRESS)) {
    /*mem= params[0]->value.memory.memory;
    start= params[1]->value.address;
    end= params[2]->value.address;*/
  }
  else
    {
      con->dd_printf("Error: wrong syntax\n"
		     "%s\n", short_help?short_help:"no help");
    }
  return(false);;
}


/*
 * Command: gui stop
 *----------------------------------------------------------------------------
 */

//int
//cl_gui_stop_cmd::do_work(class cl_sim *sim,
//			 class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_gui_stop_cmd)
{
  return(false);;
}


/* End of cmd.src/cmd_gui.cc */
