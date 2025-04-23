/*
 * Simulator of microcontrollers (cmd.src/set.cc)
 *
 * Copyright (C) 1999 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
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

//#include "ddconfig.h"

//#include <ctype.h>
#include <string.h>

//#include "i_string.h"

// prj
//#include "errorcl.h"
#include "appcl.h"

// sim
//#include "simcl.h"
//#include "optioncl.h"

// local
#include "cmd_setcl.h"
//#include "cmdutil.h"


void
set_set_help(class cl_cmd *cmd)
{
  cmd->set_help("set subcommand",
		"Set value of different objects",
		"Long of set");
}

/*
 * Command: set memory
 *----------------------------------------------------------------------------
 */

//int
//cl_set_mem_cmd::do_work(class cl_sim *sim,
//			class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_set_mem_cmd)
{
  class cl_memory *mem= 0;
  t_addr start= 0;
  t_mem *array= NULL;
  int len= 0;
  int bitnr_low= -1, bitnr_high= -1;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2) };

  if (cmdline->syntax_match(uc, BIT DATALIST))
    {
      mem= params[0]->value.bit.mem;
      start= params[0]->value.bit.mem_address;
      bitnr_low= params[0]->value.bit.bitnr_low;
      bitnr_high= params[0]->value.bit.bitnr_high;
      array= params[1]->value.data_list.array;
      len= params[1]->value.data_list.len;
    }
  else if (cmdline->syntax_match(uc, MEMORY ADDRESS DATALIST))
    {
      mem= params[0]->value.memory.memory;
      start= params[1]->value.address;
      array= params[2]->value.data_list.array;
      len= params[2]->value.data_list.len;
    }

  if (mem)
    {
      if (len == 0)
        con->dd_printf("Error: no data\n");
      else if (start < mem->get_start_address())
        {
          con->dd_printf("Start address less than ");
          con->dd_printf(mem->addr_format, mem->get_start_address());
          con->dd_printf("\n");
        }
      else if (bitnr_low >= 0)
        {
          if (len > 1)
            con->dd_printf("Error: excess data for bit(s)\n");
          else
            {
              t_mem mask= ((1U << (bitnr_high - bitnr_low + 1)) - 1) << bitnr_low;
              mem->write(start, (mem->get(start) & (~mask)) | ((array[0] << bitnr_low) & mask));
              uc->check_errors();
              mem->dump(start, bitnr_high, bitnr_low, con);
            }
        }
      else
        {
          int i;
          t_addr addr, last= mem->get_start_address()+mem->get_size()-1;
	  class cl_dump_ads ads(start, start, true, true);
          for (i= 0, addr= start;
               (i < len) &&
		 mem->valid_address(addr) &&
		 (addr <= last);
               i++, addr++)
            mem->write(addr, array[i]);
          uc->check_errors();
	  ads.stop= addr;
          mem->dump(/*start, start+len-1*/&ads, 8, con);
        }
    }
  else
    syntax_error(con);

  return(false);
}

CMDHELP(cl_set_mem_cmd,
	"set memory memory_type address data... | bit data",
	"Place list of data into memory OR set specified bit(s) to data",
	"")

/*
 * Command: set hw
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_set_hw_cmd)
{
  class cl_hw *hw= 0;
  class cl_cmd_arg *params[1]= { cmdline->param(0) };
  
  if (params[0] && params[0]->as_hw(uc)) {
    hw= params[0]->value.hw;
  }
  else
    syntax_error(con);

  if (hw)
    {
      cmdline->shift();
      if (!hw->set_cmd(cmdline, con))
	hw->set_help(con);
    }
  else
    con->dd_printf("Error: no hw\n");

  return(false);;
}

CMDHELP(cl_set_hw_cmd,
	"set hardware category params...",
	"Set parameters of specified hardware element",
	"")

/*
 * Command: set option
 *----------------------------------------------------------------------------
 */

//int
//cl_set_option_cmd::do_work(class cl_sim *sim,
//			   class cl_cmdline *cmdline, class cl_console *con)
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
	return(false);
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
	const char *cr= con->get_name();
	n= app->options->nuof_options(id, cr);
	if (n > 1)
	  con->dd_printf("Ambiguous option name, use number instead\n");
	else if (n == 0)
	    con->dd_printf("Option does not exist\n");
	else
	  option= app->options->get_option(id, cr);
      }
    else if (n == 0)
      ;//con->dd_printf("Named option does not exist\n");
    else
      option= app->options->get_option(id);
  }
  else
    syntax_error(con);
  if (!option)
    {
      con->dd_printf("Option does not exist\n");
      return(false);
    }

  option->set_value(s);

  return(false);
}

CMDHELP(cl_set_option_cmd,
	"set option name|nr value",
	"Set value of an option",
	"")
	
/*
 * Command: set error
 *----------------------------------------------------------------------------
 */

//int
//cl_set_option_cmd::do_work(class cl_sim *sim,
//			   class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_APP(cl_set_error_cmd)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };
  char *error_name= 0, *value= 0;

  if (cmdline->syntax_match(0/*app->get_uc()*/, STRING STRING)) {
    error_name= params[0]->value.string.string;
    value= params[1]->value.string.string;
  }
  else
    syntax_error(con);

  class cl_list *registered_errors = cl_error_registry::get_list();
  if (error_name &&
      value &&
      registered_errors)
    {
      int i;
      for (i= 0; i < registered_errors->count; i++)
	{
	  class cl_error_class *e=
	    (class cl_error_class *)(registered_errors->object_at(i));
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
	      return(false);
	    }
	}
    }
  con->dd_printf("Error %s not found\n", error_name);

  return(false);
}

CMDHELP(cl_set_error_cmd,
	"set error error_name on|off|unset",
	"Set value of an error",
	"")

/*
 * Command: set console
 *----------------------------------------------------------------------------
 */
COMMAND_DO_WORK_APP(cl_set_console_cmd)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };
  char *s1= 0, *s2= 0;
  class cl_uc *uc= app->sim->uc;
  if (params[0] && /*cmdline->syntax_match(uc, HW)*/params[0]->as_hw(uc))
    {
      class cl_hw *hw= params[0]->value.hw;
      con->dd_printf("Converting console to display of %s[%d]...\n",
		     hw->id_string, hw->id);
      hw->new_io(con->get_fin(), con->get_fout());
      if (hw->get_io())
	return con->drop_files(), true;
      else
	return con->dd_printf("%s[%d]: no display\n", hw->id_string, hw->id), false;
    }
  
  if (cmdline->syntax_match(0, STRING))
    s1= params[0]->value.string.string;
  else if (cmdline->syntax_match(0, STRING STRING))
    {
      s1= params[0]->value.string.string;
      s2= params[1]->value.string.string;
    }

  if (!s1)
    return false;
  
  if (strstr(s1, "i") == s1)
    {
      // interactive
      int val;
      if (!bool_name(s2, &val))
	val= 1;
      con->set_interactive(val);
    }
  else if (strstr(s1, "n") == s1)
    {
      // non-interactive
      con->set_interactive(false);
    }
  else if (strstr(s1, "r") == s1)
    {
      // raw
      con->set_cooked(false);
    }
  else if ((strstr(s1, "c") == s1) ||
	   (strstr(s1, "e") == s1))
    {
      // cooked, edited
      con->set_cooked(true);
    }
  else
    syntax_error(con);
  
  return false;
}

CMDHELP(cl_set_console_cmd,
	"set console interactive [on|off]|noninteractive|raw|edited",
	"Set console parameters",
	"")

/* End of cmd.src/cmd_set.cc */
