/*
 * Simulator of microcontrollers (brk.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#include "ddconfig.h"

#include <stdio.h>
#include <ctype.h>

#include "pobjcl.h"
#include "brkcl.h"


/*
 * Base object of breakpoints
 */

cl_brk::cl_brk(class cl_address_space *imem, int inr, t_addr iaddr,
	       enum brk_perm iperm, int ihit):
  cl_base()
{
  mem  = imem;
  nr   = inr;
  addr = iaddr;
  perm = iperm;
  hit  = ihit;
  cnt  = ihit;
}

cl_brk::~cl_brk(void)
{}

void
cl_brk::activate(void)
{
  if (mem)
    mem->set_brk(addr, this);
}

void
cl_brk::inactivate(void)
{
  if (mem)
    mem->del_brk(addr, this);
}

bool
cl_brk::do_hit(void)
{
  cnt--;
  if (cnt <= 0)
    {
      cnt= hit;
      return(1);
    }
  return(0);
}


/*
 * FETCH type of breakpoint
 */

cl_fetch_brk::cl_fetch_brk(class cl_address_space *imem, int inr, t_addr iaddr,
			   enum brk_perm iperm, int ihit):
  cl_brk(imem, inr, iaddr, iperm, ihit)
{
  code = 0;
}

enum brk_type
cl_fetch_brk::type(void)
{
  return(brkFETCH);
}


/*
 * Base of EVENT type of breakpoints
 */

cl_ev_brk::cl_ev_brk(class cl_address_space *imem, int inr, t_addr iaddr,
		     enum brk_perm iperm, int ihit,
		     enum brk_event ievent, const char *iid):
  cl_brk(imem, inr, iaddr, iperm, ihit)
{
  event= ievent;
  id   = iid;
  mem  = imem;
}

cl_ev_brk::cl_ev_brk(class cl_address_space *imem, int inr, t_addr iaddr,
		     enum brk_perm iperm, int ihit, char op):
  cl_brk(imem, inr, iaddr, iperm, ihit)
{
  mem  = imem;
  if ((op= toupper(op)) == 'R')
    {
      event= brkREAD;
      id= "read";
    }
  else if (op == 'W')
    {
      event= brkWRITE;
      id= "write";
    }
  else
    {
      event= brkACCESS;
      id= "access";
    }
}

enum brk_type
cl_ev_brk::type(void)
{
  return(brkEVENT);
}

bool
cl_ev_brk::match(struct event_rec *ev)
{
  return(DD_FALSE);
}


/*
 * Collection of break-points
 *
 * This is a sorted collection, sorted by nr field of brk items.
 */

brk_coll::brk_coll(t_index alimit, t_index adelta,
		   class cl_address_space *arom):
  cl_sorted_list(alimit, adelta, "breakpoints")
{
  rom= arom;
}

const void *
brk_coll::key_of(void *item)
{
  return((void *)&(((class cl_brk *)(item))->nr));
}


int
brk_coll::compare(const void *key1, const void *key2)
{
  int k1, k2;

  k1= *(int *)key1;
  k2= *(int *)key2;

  if (k1 == k2)
    return(0);
  else
    if (k1 < k2)
      return(-1);
    else
      return(+1);
}


/*
 * Checking if there is an event breakpoint for the specified event
 */

bool
brk_coll::there_is_event(enum brk_event ev)
{
  class cl_brk *b;
  int   i;

  for (i= 0; i < count; i++)
    {
      b= (class cl_brk *)at(i);
      if (b->type() == brkEVENT &&
	  ((class cl_ev_brk *)b)->event == ev)
	return(DD_TRUE);
    }
  return(DD_FALSE);
}

/*int
brk_coll::make_new_nr(void)
{
  if (count == 0)
    return(1);
  class cl_brk *b= (class cl_brk *)(at(count-1));
  return(b->nr+1);
}*/

void
brk_coll::add_bp(class cl_brk *bp)
{
  add(bp);
  bp->activate();
  return;
  /*if (rom &&
      bp->addr < rom->size)
      / *rom->bp_map->set(bp->addr)* /rom->set_brk(bp->addr, bp);*/
}

void
brk_coll::del_bp(t_addr addr)
{
  int idx;
  class cl_brk *bp;

  if ((bp= get_bp(addr, &idx)))
    {
      bp->inactivate();
      free_at(idx);
    }
  return;
  /*if (rom &&
      addr < rom->size)
    {
      fprintf(stderr, "brk_coll::del_bp(0x%"_A_"x\n", addr);//FIXME
      //rom->bp_map->clear(addr);
      }*/
}

void
brk_coll::del_bp(t_index idx, int /*dummy*/)
{
  class cl_brk *bp;

  if (idx >= count)
    return;
  bp= (class cl_brk *)(at(idx));
  if (!bp)
    return;
  bp->inactivate();
  free_at(idx);
}

class cl_brk *
brk_coll::get_bp(t_addr addr, int *idx)
{
  if (rom &&
      rom->valid_address(addr) &&
      rom->get_cell_flag(addr, CELL_FETCH_BRK))
    {
      for (*idx= 0; *idx < count; (*idx)++)
	{
	  class cl_brk *b= (class cl_brk *)(at(*idx));
	  if (b->addr == addr)
	    return(b);
	}
    }
  return(0);
}

class cl_brk *
brk_coll::get_bp(int nr)
{
  int i;

  for (i= 0; i < count; i++)
    {
      class cl_brk *bp= (class cl_brk *)(at(i));
      if (bp->nr == nr)
	return(bp);
    }
  return(0);
}

bool
brk_coll::bp_at(t_addr addr)
{
  return(rom &&
	 rom->valid_address(addr) &&
	 rom->get_cell_flag(addr, CELL_FETCH_BRK));
}


/* End of brk.cc */
