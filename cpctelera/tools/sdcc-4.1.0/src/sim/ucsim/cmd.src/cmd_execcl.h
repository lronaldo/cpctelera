/*
 * Simulator of microcontrollers (cmd.src/cmdsetcl.h)
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

#ifndef CMD_CMD_EXECCL_HEADER
#define CMD_CMD_EXECCL_HEADER

// local, cmd
#include "commandcl.h"
#include "newcmdcl.h"


// Execution
COMMAND_ON(sim,cl_run_cmd);
COMMAND_ON(sim,cl_stop_cmd);
COMMAND_ON(sim,cl_step_cmd);
COMMAND_ON(sim,cl_next_cmd);

//COMMAND_ON(app,cl_help_cmd);
COMMAND_HEAD(cl_help_cmd)
COMMAND_METHODS_ON(app,cl_help_cmd)
  private:
int matches;
class cl_cmd *cmd_found;
bool do_set(class cl_cmdline *cmdline, int pari, class cl_cmdset *cmdset,
	    class cl_console_base *con);
COMMAND_TAIL;

COMMAND(cl_quit_cmd);
COMMAND_ON(app,cl_kill_cmd);
COMMAND_ON(app,cl_exec_cmd);
COMMAND_ON(app,cl_expression_cmd);


// History
COMMAND_ON(uc,cl_hist_cmd);
COMMAND_ON(uc,cl_hist_info_cmd);
COMMAND_ON(uc,cl_hist_clear_cmd);
COMMAND_ON(uc,cl_hist_list_cmd);


#endif

/* End of cmd.src/cmd_execcl.h */
