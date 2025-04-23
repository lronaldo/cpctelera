/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */



// local
#include "pblazecl.h"
#include "portidcl.h"



cl_port_id::cl_port_id(class cl_uc *auc) : cl_hw(auc, HW_PORT, 0, "port_id")
{
}

/*
int
cl_port::init(void)
{
  return(0);
}
*/

bool
cl_port_id::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  class cl_cmd_arg *params[1]= { cmdline->param(0) };

  if (cmdline->syntax_match(uc, NUMBER))
    {
      value = params[0]->value.number;
      return true; // handled
    }
  return false; // unhandled
}

void
cl_port_id::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware port_id[%d] value\n"
		 "                   Set port id value value\n",
		 id);
}

void
cl_port_id::print_info(class cl_console_base *con)
{
  con->dd_printf("Port ID value: %d\n", value);
}

/* End of pblaze.src/portid.cc */
