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
#include "cmd_memcl.h"


/*
 * Command: memory create chip
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//			    class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_memory_create_chip_cmd)
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
  else if ((size < 1) ||
	   (size > max_mem_size))
    con->dd_printf("Wrong size\n");
  else if ((width < 1) ||
	   (width > 32))
    con->dd_printf("Wrong width\n");
  else
    {
      class cl_memory *mem= new cl_memory_chip(memid, size, width);
      mem->init();
      uc->memchips->add(mem);
      mem->set_uc(uc);
    }
  return(false);
}


/*
 * Command: memory create addressspace
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//			    class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_memory_create_addressspace_cmd)
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
  else if ((size < 1) ||
	   (size > max_mem_size))
    con->dd_printf("Wrong size\n");
  else if ((width < 1) ||
	   (width > 32))
    con->dd_printf("Wrong width\n");
  else
    {
      class cl_address_space *mem=
	new cl_address_space(memid, start, size, width);
      mem->init();
      uc->address_spaces->add(mem);
      mem->set_uc(uc);
    }
  return(false);
}


/*
 * Command: memory create addressdecoder
 *----------------------------------------------------------------------------
 */

//int
//cl_conf_addmem_cmd::do_work(class cl_sim *sim,
//			    class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_memory_create_addressdecoder_cmd)
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
      d->init();
      ((class cl_address_space *)as)->decoders->add(d);
      d->activate(con);
    }
  return(false);
}



/*
 * Command: memory create banker
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_memory_create_banker_cmd)
{
  class cl_cmd_arg *params[6]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3),
				 cmdline->param(4),
				 cmdline->param(5) };
  class cl_memory *banker_as= 0, *banked_as= 0;
  t_addr addr= 0, asb= 0, ase= 0;
  t_mem mask= 0;

  if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY NUMBER NUMBER)) {
    banker_as= params[0]->value.memory.memory;
    addr= params[1]->value.number;
    mask= params[2]->value.number;
    banked_as= params[3]->value.memory.memory;
    asb= params[4]->value.number;
    ase= params[5]->value.number;
  }
  else
    return con->dd_printf("Syntax error.\n"), false;

  if (!banker_as->is_address_space())
    con->dd_printf("%s is not an address space\n", banker_as->get_name("unknown"));
  else if (!banked_as->is_address_space())
    con->dd_printf("%s is not an address space\n", banked_as->get_name("unknown"));
  else if (addr < banker_as->start_address ||
           addr > banker_as->highest_valid_address())
    con->dd_printf("Specified banker address is out of address space\n");
  else if (asb < banked_as->start_address ||
           asb > banked_as->highest_valid_address())
    con->dd_printf("Specified banked start address is out of address space\n");
  else if (ase < banked_as->start_address ||
           ase > banked_as->highest_valid_address())
    con->dd_printf("Specified banked end address is out of address space\n");
  else
    {
      class cl_banker *d=
	new cl_banker((class cl_address_space *)banker_as, addr, mask, //0,
		      (class cl_address_space *)banked_as, asb, ase);
      d->init();
      ((class cl_address_space *)banked_as)->decoders->add(d);
      ((class cl_address_space *)banked_as)->undecode_area(d, asb, ase, con);
      d->activate(con);
    }
  return(false);
}


/*
 * Command: memory create bander
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_memory_create_bander_cmd)
{
  class cl_cmd_arg *params[7]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3),
				 cmdline->param(4),
				 cmdline->param(5),
				 cmdline->param(6) };
  class cl_memory *as= 0;
  t_addr asb= 0, ase= 0;
  class cl_memory *chip= 0;
  t_addr cb= 0;
  int bpc= 0, dist= 1;

  if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY NUMBER NUMBER NUMBER))
    {
      as= params[0]->value.memory.memory;
      asb= params[1]->value.number;
      ase= params[2]->value.number;
      chip= params[3]->value.memory.memory;
      cb= params[4]->value.number;
      bpc= params[5]->value.number;
      dist= params[6]->value.number;
    }
  else if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY NUMBER NUMBER))
    {
      as= params[0]->value.memory.memory;
      asb= params[1]->value.number;
      ase= params[2]->value.number;
      chip= params[3]->value.memory.memory;
      cb= params[4]->value.number;
      bpc= params[5]->value.number;
    }
  else
    return con->dd_printf("Syntax error.\n"), false;

  if (!as->is_address_space())
    con->dd_printf("%s is not an address space\n", as->get_name("unknown"));
  else if (!chip->is_chip())
    con->dd_printf("%s is not a chip\n", chip->get_name("unknown"));
  else if (asb < as->start_address ||
           asb > as->highest_valid_address())
    con->dd_printf("Specified begin address is out of address space\n");
  else if (ase < as->start_address ||
           ase > as->highest_valid_address())
    con->dd_printf("Specified end address is out of address space\n");
  else if (cb < chip->start_address ||
           cb > chip->highest_valid_address())
    con->dd_printf("Specified chip address is out of size\n");
  else
    {
      class cl_bander *b=
	new cl_bander((class cl_address_space *)as, asb, ase,
		      (class cl_memory_chip *)chip, cb,
		      bpc, dist);
      b->init();
      ((class cl_address_space *)as)->decoders->add(b);
      b->activate(con);
    }
  return(false);
}


/*
 * Command: memory create bank
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_memory_create_bank_cmd)
{
  class cl_cmd_arg *params[5]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3),
				 cmdline->param(4) };
  class cl_memory *as= 0, *chip= 0;
  t_addr as_begin= 0, chip_begin= 0;
  int bank= -1;
  
  if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY NUMBER)) {
    as= params[0]->value.memory.memory;
    as_begin= params[1]->value.number;
    bank= params[2]->value.number;
    chip= params[3]->value.memory.memory;
    chip_begin= params[4]->value.number;
  }
  else if (cmdline->syntax_match(uc, MEMORY NUMBER NUMBER MEMORY)) {
    as= params[0]->value.memory.memory;
    as_begin= params[1]->value.number;
    bank= params[2]->value.number;
    chip= params[3]->value.memory.memory;
  }
  else
    return con->dd_printf("Syntax error.\n"), false;

  if (!as->is_address_space())
    con->dd_printf("%s is not an address space\n", as->get_name("unknown"));
  else if (!chip->is_chip())
    con->dd_printf("%s is not a memory chip\n", chip->get_name("unknown"));
  else if (chip_begin >= chip->get_size())
    con->dd_printf("Wrong chip area specification\n");
  else if (as_begin < as->start_address ||
           as_begin > as->highest_valid_address())
    con->dd_printf("Specified area is out of address space\n");
  else
    {
      class cl_banker *d=
	(class cl_banker *)((class cl_address_space *)as)->get_decoder_of(as_begin);
      if (!d)
	con->dd_printf("Specified address is not decoded, create a banker first\n");
      else if (!d->is_banker())
	con->dd_printf("Specified area is not decoded by banker\n");
      else
	{
	  d->add_bank(bank, chip, chip_begin);
	}
    }
  return(false);
}


/*
 * Command: memory cell
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_memory_cell_cmd)
{
  class cl_cmd_arg *params[5]= { cmdline->param(0),
				 cmdline->param(1) };
  class cl_memory *m= 0;
  t_addr a= 0;
  class cl_address_space *as= 0;
  class cl_memory_cell *c= 0;

  if (cmdline->syntax_match(uc, CELL))
    {
      c= params[0]->value.cell;
      m= as= uc->address_space(c, &a);
    }
  else if (cmdline->syntax_match(uc, MEMORY ADDRESS /*NUMBER*/))
    {
      m= params[0]->value.memory.memory;
      a= params[1]->value.number;
      if (m->is_address_space())
	as= (cl_address_space *)m;
    }
  if (m == 0)
    return con->dd_printf("Syntax error.\n"), false;

  if (!c)
    c= as->get_cell(a);
  con->dd_printf("%s", as->get_name());
  con->dd_printf("[");
  con->dd_printf(as->addr_format, a);
  con->dd_printf("] %s\n", (char*)uc->cell_name(c));

  con->dd_printf("cell data=%p/%d mask=%x flags=%x\n",
		 c->get_data(),
		 c->get_width(),
		 c->get_mask(),
		 c->get_flags());

  int i;
  for (i= 0; i < uc->memchips->count; i++)
    {
      cl_memory_chip *ch= (cl_memory_chip*)(uc->memchips->at(i));
      t_addr ad;
      if ((ad= ch->is_slot(c->get_data())) >= 0)
	{
	  con->dd_printf("  decoded to %s[%d]\n",
			 ch->get_name(), ad);
	  break;
	}
    }

  con->dd_printf("Operators:\n");
  c->print_operators(" ", con);
  
  return false;
}


/* End of cmd.src/cmd_mem.cc */
