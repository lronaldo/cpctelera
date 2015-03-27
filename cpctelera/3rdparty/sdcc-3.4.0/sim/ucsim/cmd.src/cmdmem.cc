/*
 * Simulator of microcontrollers (cmd.src/cmdmem.cc)
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
#include "cmdmemcl.h"


/*
 * Command: memory createchip
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//                          class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_memory_createchip_cmd)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };
  char *memid= NULL;
  int size= -1;
  int width= 8;

  if (cmdline->syntax_match(uc, STRING NUMBER)) {
    memid= params[0]->value.string.string;
    size= params[1]->value.number;
  }
  else if (cmdline->syntax_match(uc, STRING NUMBER NUMBER)) {
    memid= params[0]->value.string.string;
    size= params[1]->value.number;
    width= params[2]->value.number;
  }
  else
    con->dd_printf("Syntax error.\n");

  if (!memid ||
      !*memid)
    con->dd_printf("Wrong id\n");
  else if (size < 1)
    con->dd_printf("Wrong size\n");
  else
    {
      class cl_memory *mem= new cl_memory_chip(memid, size, width);
      mem->init();
      uc->memchips->add(mem);
      mem->set_uc(uc);
    }
  return(DD_FALSE);
}


/*
 * Command: memory createaddressspace
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//                          class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_memory_createaddressspace_cmd)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };
  char *memid= NULL;
  int start= 0, size= -1, width= 8;

  if (cmdline->syntax_match(uc, STRING NUMBER)) {
    memid= params[0]->value.string.string;
    size= params[1]->value.number;
  }
  else if (cmdline->syntax_match(uc, STRING NUMBER NUMBER)) {
    memid= params[0]->value.string.string;
    start= params[1]->value.number;
    size= params[2]->value.number;
  }
  else if (cmdline->syntax_match(uc, STRING NUMBER NUMBER NUMBER)) {
    memid= params[0]->value.string.string;
    start= params[1]->value.number;
    size= params[2]->value.number;
    width= params[3]->value.number;
  }
  else
    con->dd_printf("Syntax error.\n");

  if (!memid ||
      !*memid)
    con->dd_printf("Wrong id\n");
  else if (size < 1)
    con->dd_printf("Wrong size\n");
  else
    {
      class cl_address_space *mem=
        new cl_address_space(memid, start, size, width);
      mem->init();
      uc->address_spaces->add(mem);
      mem->set_uc(uc);
    }
  return(DD_FALSE);
}


/*
 * Command: memory createaddressdecoder
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//                          class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_memory_createaddressdecoder_cmd)
{
  class cl_cmd_arg *params[5]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3),
                                 cmdline->param(4) };
  class cl_memory *as= 0, *chip= 0;
  t_addr as_begin= 0, as_end= 0, chip_begin= 0;

  if (cmdline->syntax_match(uc, MEMORY MEMORY)) {
    as= params[0]->value.memory.memory;
    as_end= as->highest_valid_address();
    chip= params[1]->value.memory.memory;
  }
  else if (cmdline->syntax_match(uc, MEMORY MEMORY NUMBER)) {
    as= params[0]->value.memory.memory;
    as_end= as->highest_valid_address();
    chip= params[1]->value.memory.memory;
    chip_begin= params[2]->value.number;
  }
  else if (cmdline->syntax_match(uc, MEMORY NUMBER MEMORY)) {
    as= params[0]->value.memory.memory;
    as_begin= params[1]->value.number;
    as_end= as->highest_valid_address();
    chip= params[2]->value.memory.memory;
  }
  else if (cmdline->syntax_match(uc, MEMORY NUMBER MEMORY NUMBER)) {
    as= params[0]->value.memory.memory;
    as_begin= params[1]->value.number;
    as_end= as->highest_valid_address();
    chip= params[2]->value.memory.memory;
    chip_begin= params[3]->value.number;
  }
  else if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY)) {
    as= params[0]->value.memory.memory;
    as_begin= params[1]->value.number;
    as_end= params[2]->value.number;
    chip= params[3]->value.memory.memory;
  }
  else if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY NUMBER)) {
    as= params[0]->value.memory.memory;
    as_begin= params[1]->value.number;
    as_end= params[2]->value.number;
    chip= params[3]->value.memory.memory;
    chip_begin= params[4]->value.number;
  }
  else
    con->dd_printf("Syntax error.\n");

  if (!as->is_address_space())
    con->dd_printf("%s is not an address space\n", as->get_name("unknown"));
  else if (!chip->is_chip())
    con->dd_printf("%s is not a memory chip\n", chip->get_name("unknown"));
  else if (as_begin > as_end)
    con->dd_printf("Wrong address area specification\n");
  else if (chip_begin >= chip->get_size())
    con->dd_printf("Wrong chip area specification\n");
  else if (as_begin < as->start_address ||
           as_end > as->highest_valid_address())
    con->dd_printf("Specified area is out of address space\n");
  else if (as_end-as_begin > chip->get_size()-chip_begin)
    con->dd_printf("Specified area is out of chip size\n");
  else
    {
      class cl_address_decoder *d=
        new cl_address_decoder(as, chip, as_begin, as_end, chip_begin);
      ((class cl_address_space *)as)->decoders->add(d);
      d->activate(con);
    }
  return(DD_FALSE);
}


/* End of cmd.src/cmdmem.cc */
