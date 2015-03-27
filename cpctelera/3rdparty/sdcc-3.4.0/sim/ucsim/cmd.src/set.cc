/*
 * Simulator of microcontrollers (cmd.src/set.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#include "ddconfig.h"

#include <ctype.h>
#include "i_string.h"

// prj
#include "errorcl.h"

// sim
#include "simcl.h"
#include "optioncl.h"

// local
#include "setcl.h"
#include "cmdutil.h"


/*
 * Command: set memory
 *----------------------------------------------------------------------------
 */

//int
//cl_set_mem_cmd::do_work(class cl_sim *sim,
//                      class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_set_mem_cmd)
{
  class cl_memory *mem= 0;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };

  if (cmdline->syntax_match(uc, MEMORY ADDRESS DATALIST)) {
    mem= params[0]->value.memory.memory;
    t_addr start= params[1]->value.address;
    t_mem *array= params[2]->value.data_list.array;
    int len= params[2]->value.data_list.len;
    
    if (len == 0)
      con->dd_printf("Error: no data\n");
    else if (start < mem->get_start_address())
      con->dd_printf("Start address less then 0x%"_A_"x\n",
                     mem->get_start_address());
    else
      {
        int i;
        t_addr addr;
        for (i= 0, addr= start;
             i < len && mem->valid_address(addr);
             i++, addr++)
          mem->write(addr, array[i]);
        uc->check_errors();
        mem->dump(start, start+len-1, 8, con);
      }
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");
  
  return(DD_FALSE);;
}


/*
 * Command: set bit
 *----------------------------------------------------------------------------
 */

//int
//cl_set_bit_cmd::do_work(class cl_sim *sim,
//                      class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_set_bit_cmd)
{
  class cl_memory *mem;
  t_addr mem_addr= 0;
  t_mem bit_mask= 0;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };
  
  if (cmdline->syntax_match(uc, BIT NUMBER)) {
    mem= params[0]->value.bit.mem;
    mem_addr= params[0]->value.bit.mem_address;
    bit_mask= params[0]->value.bit.mask;
    if (params[1]->value.number)
      mem->set_bit1(mem_addr, bit_mask);
    else
      mem->set_bit0(mem_addr, bit_mask);
    mem->dump(mem_addr, mem_addr, 1, con);
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");

  return(DD_FALSE);;
}


/*
 * Command: set hw
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_set_hw_cmd)
{
  class cl_hw *hw= 0;
  class cl_cmd_arg *params[1]= { cmdline->param(0)/*,
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3)*/ };
  
  if (params[0] && /*cmdline->syntax_match(uc, HW)*/params[0]->as_hw(uc)) {
    hw= params[0]->value.hw;
    //pn= hw->id;
    //l= params[1]->value.number;
  }
  /*else if (cmdline->syntax_match(uc, NUMBER NUMBER)) {
    pn= params[0]->value.number;
    l= params[1]->value.number;
    hw= uc->get_hw(HW_PORT, pn, 0);
    }*/
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");
  /*if (pn < 0 ||
      pn > 3)
    con->dd_printf("Error: wrong port\n");
    else*/
    {
      if (hw)
        {
          cmdline->shift();
          hw->set_cmd(cmdline, con);
        }
      else
        con->dd_printf("Error: no hw\n");
    }
  return(DD_FALSE);;
}


/*
 * Command: set option
 *----------------------------------------------------------------------------
 */

//int
//cl_set_option_cmd::do_work(class cl_sim *sim,
//                         class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_APP(cl_set_option_cmd)
{
  char *id= 0, *s= 0;
  int idx;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };
  class cl_option *option= 0;

  if (cmdline->syntax_match(0/*app->get_uc()*/, NUMBER STRING)) {
    idx= params[0]->value.number;
    s= params[1]->value.string.string;
    option= app->options->get_option(idx);
  }
  else if (cmdline->syntax_match(0, STRING STRING STRING)) {
    id= params[0]->value.string.string;
    char *cr= params[1]->value.string.string;
    s= params[2]->value.string.string;
    int n= app->options->nuof_options(id, cr);
    if (n > 1)
      {
        con->dd_printf("Ambiguous option name, use number instead\n");
        return(DD_FALSE);
      }
    else if (n == 0)
      ;//con->dd_printf("Named option does not exist\n");
    else
      {
        if ((option= app->options->get_option(id, cr)) == 0)
          option= app->options->get_option(cr, id);
      }
  }
  else if (cmdline->syntax_match(0/*app->get_uc()*/, STRING STRING)) {
    id= params[0]->value.string.string;
    s= params[1]->value.string.string;
    int n= app->options->nuof_options(id);
    if (n > 1)
      {
        con->dd_printf("Ambiguous option name, use number instead\n");
        return(DD_FALSE);
      }
    else if (n == 0)
      ;//con->dd_printf("Named option does not exist\n");
    else
      option= app->options->get_option(id);
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");
  if (!option)
    {
      con->dd_printf("Option does not exist\n");
      return(DD_FALSE);
    }

  option->set_value(s);

  return(DD_FALSE);
}


/*
 * Command: set error
 *----------------------------------------------------------------------------
 */

//int
//cl_set_option_cmd::do_work(class cl_sim *sim,
//                         class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_APP(cl_set_error_cmd)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };
  char *error_name= NIL, *value= NIL;

  if (cmdline->syntax_match(0/*app->get_uc()*/, STRING STRING)) {
    error_name= params[0]->value.string.string;
    value= params[1]->value.string.string;
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");

  class cl_list *registered_errors = cl_error_registry::get_list();
  if (error_name &&
      value &&
      registered_errors)
    {
      int i;
      for (i= 0; i < registered_errors->count; i++)
        {
          class cl_error_class *e=
            dynamic_cast<class cl_error_class *>(registered_errors->object_at(i));
          if (e->is_inamed(error_name))
            {
              if (strchr("uU-?", *value) != NULL)
                e->set_on(ERROR_PARENT);
              else if (strchr("1tTyY", *value) != NULL ||
                       (strlen(value) > 1 &&
                        strchr("nN", value[2]) != NULL))
                e->set_on(ERROR_ON);
              else if (strchr("0fFnN", *value) != NULL ||
                       (strlen(value) > 1 &&
                        strchr("fF", value[2]) != NULL))
                e->set_on(ERROR_OFF);
              else
                con->dd_printf("Bad value (%s)\n", value);
              return(DD_FALSE);
            }
        }
    }
  con->dd_printf("Error %s not found\n", error_name);

  return(DD_FALSE);
}


/* End of cmd.src/set.cc */
