/*
 * Simulator of microcontrollers (stm8.src/flash.cc)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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

#include "flashcl.h"


cl_flash::cl_flash(class cl_uc *auc, t_addr abase, const char *aname):
	cl_hw(auc, HW_FLASH, 0, aname)
{
  base= abase;
  set_name(aname);
}

int
cl_flash::init(void)
{
  cl_hw::init();
  registration();
  reset();
  return 0;
}

void
cl_flash::reset(void)
{
  puk1st= false;
  duk1st= false;
  p_unlocked= false;
  d_unlocked= false;
  p_failed= false;
  d_failed= false;
}

t_mem
cl_flash::read(class cl_memory_cell *cell)
{
  t_mem v= cell->get();

  if (cell == pukr)
    v= 0;
  else if (cell == dukr)
    v= 0;
  else if (cell == iapsr)
    {
      v&= ~0x0a;
      if (p_unlocked)
	v|= 0x02;
      if (d_unlocked)
	v|= 0x08;
      // read clears EOP and WR_PG_DIS bits
      cell->set(v & ~0x05);
    }
  return v;
}

void
cl_flash::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;

  if (conf(cell, NULL))
    return;
  
  if (cell == pukr)
    {
      if (p_failed)
	;
      else if (!puk1st)
	{
	  if (*val == PMASS1)
	    puk1st= true;
	  else
	    p_failed= true;
	}
      else
	{
	  if (*val == PMASS2)
	    puk1st= false, p_unlocked= true;
	  else
	    puk1st= false, p_failed= true;
	}
      *val= 0;
    }
  else if (cell == dukr)
    {
      if (d_failed)
	;
      else if (!duk1st)
	{
	  if (*val == DMASS1)
	    duk1st= true;
	  else
	    d_failed= true;
	}
      else
	{
	  if (*val == DMASS2)
	    duk1st= false, d_unlocked= true;
	  else
	    duk1st= false, d_failed= true;
	}
      *val= 0;
    }
  else if (cell == iapsr)
    {
      if (!(*val & 0x02))
	p_unlocked= puk1st= false;
      if (!(*val & 0x08))
	d_unlocked= duk1st= false;
      *val&= ~0x0a;
      if (p_unlocked)
	*val|= 0x02;
      if (d_unlocked)
	*val|= 0x08;
    }
}

t_mem
cl_flash::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum stm8_flash_cfg)addr)
    {
    case stm8_flash_on:
      if (val)
	{
	  if (*val)
	    on= true;
	  else
	    on= false;
	}
      else
	cell->set(on?1:0);
      break;
    case stm8_flash_nuof_cfg:
      break;
    }
  return cell->get();
}

void
cl_flash::print_info(class cl_console_base *con)
{
  con->dd_printf(chars("", "Flash at %s\n", uc->rom->addr_format), base);
  con->dd_printf("PUK: ");
  if (p_failed)
    con->dd_printf("fail");
  else if (p_unlocked)
    con->dd_printf("unlocked");
  else if (puk1st)
    con->dd_printf("MASS1");
  else
    con->dd_printf("locked");
  con->dd_printf("\n");

  con->dd_printf("DUK: ");
  if (d_failed)
    con->dd_printf("fail");
  else if (d_unlocked)
    con->dd_printf("unlocked");
  else if (duk1st)
    con->dd_printf("MASS1");
  else
    con->dd_printf("locked");
  con->dd_printf("\n");
}


/* SAF */

cl_saf_flash::cl_saf_flash(class cl_uc *auc, t_addr abase):
	cl_flash(auc, abase, "flash")
{
}

void
cl_saf_flash::registration(void)
{
  pukr= register_cell(uc->rom, base+8);
  dukr= register_cell(uc->rom, base+10);
  iapsr= register_cell(uc->rom, base+5);
}


/* L,L101 */

cl_l_flash::cl_l_flash(class cl_uc *auc, t_addr abase):
	cl_flash(auc, abase, "flash")
{
}

void
cl_l_flash::registration(void)
{
  pukr= register_cell(uc->rom, base+2);
  dukr= register_cell(uc->rom, base+3);
  iapsr= register_cell(uc->rom, base+4);
}


/* End of stm8.src/flash.cc */
