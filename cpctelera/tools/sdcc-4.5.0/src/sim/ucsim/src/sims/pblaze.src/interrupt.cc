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


// sim
#include "itsrccl.h"

// local
#include "pblazecl.h"
#include "interruptcl.h"
#include "regspblaze.h"



cl_interrupt::cl_interrupt(class cl_uc *auc) : cl_hw(auc, HW_INTERRUPT, 0, "irq")
{
}

int
cl_interrupt::init(void)
{
  interrupt_request = false;
  return(0);
}

bool
cl_interrupt::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  class cl_cmd_arg *params[1]= { cmdline->param(0) };

  if (cmdline->syntax_match(uc, NUMBER))
    {
      interrupt_request = params[0]->value.number != 0 ? true : false;
      return true; // handled
    }
  return false; // unhandled
}

void
cl_interrupt::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware irq[%d] irq_value\n"
		 "Set interrupt request value\n",
		 id);
}

void
cl_interrupt::print_info(class cl_console_base *con)
{
  con->dd_printf("Interrupt request %s, interrupts %s\n", (interrupt_request ? "active" : "inactive"), ((class cl_pblaze *) uc)->sfr->get(FLAGS) & bmI ? "enabled":"disabled");
}

/* End of pblaze.src/interrupt.cc */
