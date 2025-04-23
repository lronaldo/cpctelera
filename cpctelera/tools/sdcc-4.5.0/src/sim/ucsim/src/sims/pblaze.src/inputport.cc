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


#include <map>
#include <list>

// local
#include "pblazecl.h"
#include "inputportcl.h"



cl_input_port::cl_input_port(class cl_uc *auc) : cl_hw(auc, HW_PORT, 0, "input_port")
{
  value_set = false;
}

bool
cl_input_port::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  class cl_cmd_arg *params[1]= { cmdline->param(0) };

  if (cmdline->syntax_match(uc, NUMBER))
    {
      value = params[0]->value.number;
      value_set = true;
      return true; // handled
    }
  return false; // unhandled
}

void
cl_input_port::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware port[%d] value\n"
		 "                   Set port value value\n",
		 id);
}

void
cl_input_port::print_info(class cl_console_base *con)
{
  con->dd_printf("Value on input port: %2X\n", value);
}

void
cl_input_port::add_input(t_mem value, int port, long tick) {

  if (inputs.find(port) == inputs.end()) {
    inputs.insert(make_pair(port, input_tick_map()));
  }
  //input_tick_map ticks = inputs[port];

  if (inputs[port].find(tick) == inputs[port].end()) {
    inputs[port].insert(make_pair(tick, list<t_mem>() ));
  }

  inputs[port][tick].push_back(value);
}

t_mem
cl_input_port::get_input(int port, long tick) {
  if (value_set) {
    value_set = false;
    return value;
  }

  int p = port;
  long t = tick;
  // check for port:tick
  if (!contains_input(p, t)) {
    // not found, check for port:-1
    if (!contains_input(p, -1)) {
      // not found, check for -1:tick
      p = -1;
      if (!contains_input(p, t)) {
        // not found, check for -1:-1
        t = -1;
        if (!contains_input(p, t)) {
          // no input entry found
          fprintf(stderr, "No input specified (port: %d, tick: %ld\n", port, tick);
          return 0;
        }
      }
    }
    else {
      t = -1;
    }
  }

  t_mem value = inputs[p][t].front();
  inputs[p][t].pop_front();

  if (inputs[p][t].empty()) {
    inputs[p].erase(t);
  }
  if (inputs[p].empty()) {
    inputs.erase(p);
  }

// print inputs
  for(input_port_map::iterator it = inputs.begin(); it != inputs.end(); it++) {
    application->debug("port: %d\n", it->first);

    for(input_tick_map::iterator it_inner = it->second.begin(); it_inner != it->second.end(); it_inner++) {
      application->debug("\ttick: %ld\n", it_inner->first);

      for(list<t_mem>::iterator it_list = it_inner->second.begin(); it_list != it_inner->second.end(); it_list++) {
        application->debug("\t\t: %d\n", *it_list);
      }
    }
  }

  return value;
}

bool
cl_input_port::contains_input(int port, long tick) {
  // check for port
  if (inputs.find(port) == inputs.end()) {
    return false;
  }

  // check for tick
  if (inputs[port].find(tick) == inputs[port].end()) {
    return false;
  }

  return true;
}

/* End of pblaze.src/inputport.cc */
