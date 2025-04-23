/*
 * Simulator of microcontrollers (itsrc.cc)
 *
 * Copyright (C) 1999 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
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

#include <stdio.h>
//#include <stdlib.h>
//#include "i_string.h"

#include "globals.h"

#include "itsrccl.h"
//#include "pobjcl.h"
//#include "stypes.h"
//#include "memcl.h"


/*
 * Interrupt source
 ******************************************************************************
 */

cl_it_src::cl_it_src(cl_uc  *Iuc,
		     int    Inuof,
		     class  cl_memory_cell *Iie_cell,
		     t_mem  Iie_mask,
		     class  cl_memory_cell *Isrc_cell,
		     t_mem  Isrc_mask,
		     t_addr Iaddr,
		     bool   Iclr_bit,
		     bool   Iindirect,
		     const  char *Iname,
		     int    apoll_priority):
	  /*cl_base()*/
	  cl_hw(Iuc, HW_INTERRUPT, 100+Inuof, chars("", "itsrc_%d", Inuof))
{
  uc= Iuc;
  poll_priority= apoll_priority;
  nuof     = Inuof;
  cid      = 0;
  ie_cell  = Iie_cell;
  ie_mask  = Iie_mask;
  ie_value = Iie_mask;
  src_cell = Isrc_cell;
  src_mask = Isrc_mask;
  src_value= Isrc_mask;
  addr     = Iaddr;
  clr_bit  = Iclr_bit;
  indirect = Iindirect;
  if (Iname != NULL)
    set_name(Iname);
  else
    set_name("unknown");
  active= true;
  parent= NULL;
}

cl_it_src::~cl_it_src(void) {}

int
cl_it_src::init(void)
{
  register_cell(ie_cell);
  register_cell(src_cell);
  return 0;
}

bool
cl_it_src::is_active(void)
{
  return(active);
}

void
cl_it_src::set_active_status(bool Aactive)
{
  active= Aactive;
}

void
cl_it_src::activate(void)
{
  set_active_status(true);
}

void
cl_it_src::deactivate(void)
{
  set_active_status(false);
}

void
cl_it_src::pass_over(void)
{
  if (is_slave())
    {
      if (pending() && enabled())
	{
	  parent->request();
	  clear();
	}
    }
}


bool
cl_it_src::enabled(void)
{
  if (!ie_cell)
    return false;
  t_mem e= ie_cell->get();
  e&= ie_mask;
  return e == ie_value;
}

bool
cl_it_src::pending(void)
{
  if (!src_cell)
    return false;
  t_mem s= src_cell->get();
  s&= src_mask;
  return s == src_value;
}

void
cl_it_src::request(void)
{
  if (!src_cell)
    return;
  if (src_value)
    src_cell->write(src_cell->get() | src_mask);
  else
    src_cell->write(src_cell->get() & ~src_mask);
}

void
cl_it_src::clear(void)
{
  if (!src_cell)
    return;
  if (clr_bit)
    {
      if (src_value)
        src_cell->write(src_cell->get() & ~src_mask);
      else
        src_cell->write(src_cell->get() | src_mask);
    }
}

void
cl_it_src::write(class cl_memory_cell *cell, t_mem *val)
{
  t_mem iev= ie_cell->get();
  t_mem srcv= src_cell->get();
  t_mem ier, srcr;
  
  if (cell == ie_cell)
    {
      iev= *val;
    }
  if (cell == src_cell)
    {
      srcv= *val;
    }
  ier= (iev&ie_mask) == ie_value;
  srcr= (srcv&src_mask) == src_value;

  if (!is_slave())
    {
      if (ier)
	{
	  if (srcr)
	    {
	      uc->irq= true;
	    }
	}
    }
}

t_mem
cl_it_src::read(class cl_memory_cell *cell)
{
  return cell->get();
}


/*
 *  Sorted list of IRQ sources
 */

cl_irqs::cl_irqs(t_index alimit, t_index adelta):
  cl_sorted_list(alimit, adelta, "irqs")
{
  Duplicates= true;
}

const void *
cl_irqs::key_of(const void *item) const
{
  const class cl_it_src *itsrc= (const class cl_it_src *)item;
  return(&itsrc->poll_priority);
}

int
cl_irqs::compare(const void *key1, const void *key2)
{
  int k1= *(const int *)key1;
  int k2= *(const int *)key2;

  if (k1 == k2)
    return(0);
  else if (k1 < k2)
    return(-1);
  return(1);
}


/*
 * Interrupt level
 ******************************************************************************
 */

it_level::it_level(int alevel, uint aaddr, uint aPC, class cl_it_src *is):
  cl_base()
{
  level = alevel;
  addr  = aaddr;
  PC    = aPC;
  source= is;
}



/* End of sim.src/itsrc.cc */
