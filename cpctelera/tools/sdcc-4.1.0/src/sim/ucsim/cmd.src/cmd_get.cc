/*
 * Simulator of microcontrollers (cmd.src/get.cc)
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

//#include "ddconfig.h"

//#include <ctype.h>
#include <string.h>

//#include "i_string.h"

//#include <stdarg.h>
// prj
#include "utils.h"
#include "appcl.h"

// sim
//#include "simcl.h"
//#include "optioncl.h"

// local
#include "cmd_getcl.h"
//#include "cmdutil.h"


void
set_get_help(class cl_cmd *cmd)
{
  cmd->set_help("get subcommand",
		"Get value of different objects",
		"Long of get");
}

/*
 * Command: get sfr
 *----------------------------------------------------------------------------
 */

//int
//cl_get_sfr_cmd::do_work(class cl_sim *sim,
//			class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_get_sfr_cmd)
{
  class cl_address_space *mem= uc->address_space(MEM_SFR_ID);
  class cl_cmd_arg *parm;
  int i;

  if (!mem)
    {
      con->dd_printf("Error: No SFR\n");
      return(false);
    }
  for (i= 0, parm= cmdline->param(i);
       parm;
       i++, parm= cmdline->param(i))
    {
      if (!parm->as_address(uc) ||
	  !mem->valid_address(parm->value.address))
	con->dd_printf("Warning: Invalid address %s\n",
		       cmdline->tokens->at(i));
      else
	mem->dump(parm->value.address, parm->value.address, 1, con/*->get_fout()*/);
    }

  return(false);;
}

CMDHELP(cl_get_sfr_cmd,
	"get sfr address...",
	"Get value of addressed SFRs",
	"long help of get sfr")

/*
 * Command: get option
 *----------------------------------------------------------------------------
 */

//int
//cl_get_option_cmd::do_work(class cl_sim *sim,
//			   class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_APP(cl_get_option_cmd)
{
  class cl_cmd_arg *parm= cmdline->param(0);
  char *s= 0;

  if (!parm)
    ;
  else if (cmdline->syntax_match(0/*app->get_uc()*/, STRING)) {
    s= parm->value.string.string;
  }
  else
    syntax_error(con);

  int i;
  for (i= 0; i < app->options->count; i++)
    {
      class cl_option *o= (class cl_option *)(/*uc*/app->options->at(i));
      if ((!s ||
	   !strcmp(s, o->get_name())))
	{
	  if (!o->hidden)
	    {
	      con->dd_printf("%2d. %s(by %s): ", i, object_name(o),
			     object_name(o->get_creator()));
	      o->print(con);
	      con->dd_printf(" - %s\n", o->help);
	    }
	  else
	    {
	      /*
	      con->dd_printf("%2d. %s(by %s) is hidden!\n", i, object_name(o),
			   object_name(o->get_creator()));
	      */
	    }
	}
    }
  
  return(false);;
}

CMDHELP(cl_get_option_cmd,
	"get option name",
	"Get value of an option",
	"long help of get option")

/* End of cmd.src/cmd_get.cc */
