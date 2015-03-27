/*
 * Simulator of microcontrollers (cmd.src/cmdconf.cc)
 *
 * Copyright (C) 2001,01 Drotos Daniel, Talker Bt.
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

// prj
#include "globals.h"
#include "utils.h"

// sim
#include "simcl.h"

// local
#include "cmdconfcl.h"


/*
 * Command: conf
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_conf_cmd)
{
  int i;

  con->dd_printf("ucsim version %s\n", VERSIONSTR);
  con->dd_printf("Type of microcontroller: %s\n", uc->id_string());
  con->dd_printf("Controller has %d hardware element(s).\n",
                 uc->hws->count);
  for (i= 0; i < uc->hws->count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(uc->hws->at(i));
      con->dd_printf("  %s[%d]\n", hw->id_string, hw->id);
    }
  return(0);
}

/*
 * Command: conf objects
 *----------------------------------------------------------------------------
 */

static void
conf_objects_cmd_print_node(class cl_console_base *con,
                            int indent, class cl_base *node)
{
  if (!node)
    return;
  int i;
  for (i= 0; i < indent; i++)
    con->dd_printf(" ");
  const char *name= node->get_name("unknown");
  con->dd_printf("%s\n", name);
  class cl_base *c= node->first_child();
  while (c)
    {
      conf_objects_cmd_print_node(con, indent+2, c);
      c= node->next_child(c);
    }
}

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//                          class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_APP(cl_conf_objects_cmd)
{
  //class cl_address_space *mem= 0;
  /*class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };*/
  class cl_error *cl_error_base = new cl_error();
  conf_objects_cmd_print_node(con, 0, /*application*/cl_error_base->get_class());
  delete cl_error_base;
  return(DD_FALSE);
}


/* End of cmd.src/cmdconf.cc */
