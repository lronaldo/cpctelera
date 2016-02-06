/*
 * Simulator of microcontrollers (hw.cc)
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

#include <stdlib.h>
#include "i_string.h"

#include "stypes.h"
#include "hwcl.h"


/*
 *____________________________________________________________________________
 */

cl_watched_cell::cl_watched_cell(class cl_address_space *amem, t_addr aaddr,
                                 class cl_memory_cell **astore,
                                 enum what_to_do_on_cell_change awtd)
{
  mem= amem;
  addr= aaddr;
  store= astore;
  wtd= awtd;
  if (mem)
    {
      cell= mem->get_cell(addr);
      if (store)
        *store= cell;
    }
}

void
cl_watched_cell::mem_cell_changed(class cl_address_space *amem, t_addr aaddr,
                                  class cl_hw *hw)
{
  if (mem &&
      mem == amem &&
      addr == aaddr)
    {
      cell= mem->get_cell(addr);
      if (store &&
          (wtd & WTD_RESTORE))
        *store= cell;
      if (wtd & WTD_WRITE)
        {
          t_mem d= cell->get();
          hw->write(cell, &d);
        }
    }
}

void
cl_watched_cell::address_space_added(class cl_address_space *amem,
                                     class cl_hw *hw)
{
}

void
cl_used_cell::mem_cell_changed(class cl_address_space *amem, t_addr aaddr, 
class cl_hw *hw)
{
  if (mem &&
      mem == amem &&
      addr == aaddr)
    {
      cell= mem->get_cell(addr);
      if (store &&
          (wtd & WTD_RESTORE))
        *store= cell;
      if (wtd & WTD_WRITE)
        {
          t_mem d= cell->get();
          hw->write(cell, &d);
        }
    }
}

void
cl_used_cell::address_space_added(class cl_address_space *amem,
                                  class cl_hw *hw)
{
}


/*
 *____________________________________________________________________________
 */

cl_hw::cl_hw(class cl_uc *auc, enum hw_cath cath, int aid, const char *aid_string):
  cl_guiobj()
{
  flags= HWF_INSIDE;
  uc= auc;
  cathegory= cath;
  id= aid;
  if (aid_string &&
      *aid_string)
    id_string= strdup(aid_string);
  else
    id_string= strdup("unknown hw element");
  char *s= (char*)malloc(strlen(get_name("hw"))+100);
  sprintf(s, "partners of %s", get_name("hw"));
  partners= new cl_list(2, 2, s);
  sprintf(s, "watched cells of %s", get_name("hw"));
  watched_cells= new cl_list(2, 2, s);
  free(s);
}

cl_hw::~cl_hw(void)
{
  free(id_string);
  //hws_to_inform->disconn_all();
  delete partners;
  delete watched_cells;
}


void
cl_hw::new_hw_adding(class cl_hw *new_hw)
{
}

void
cl_hw::new_hw_added(class cl_hw *new_hw)
{
  int i;

  for (i= 0; i < partners->count; i++)
    {
      class cl_partner_hw *ph= (class cl_partner_hw *)(partners->at(i));
      ph->refresh(new_hw);
    }
}

class cl_hw *
cl_hw::make_partner(enum hw_cath cath, int id)
{
  class cl_partner_hw *ph;
  class cl_hw *hw;

  ph= new cl_partner_hw(uc, cath, id);
  partners->add(ph);
  hw= ph->get_partner();
  return(hw);
}


/*
 * Callback functions for changing memory locations
 */

/*t_mem
cl_hw::read(class cl_m *mem, t_addr addr)
{
  // Simply return the value
  return(mem->get(addr));
}*/

/*void
cl_hw::write(class cl_m *mem, t_addr addr, t_mem *val)
{
  // Do not change *val by default
}*/

void
cl_hw::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  con->dd_printf("Nothing to do\n");
}

class cl_memory_cell *
cl_hw::register_cell(class cl_address_space *mem, t_addr addr,
                     class cl_memory_cell **store,
                     enum what_to_do_on_cell_change awtd)
{
  class cl_watched_cell *wc;
  class cl_memory_cell *cell;

  if (mem)
    mem->register_hw(addr, this, (int*)0, DD_FALSE);
  else
    printf("regcell JAJ no mem\n");
  wc= new cl_watched_cell(mem, addr, &cell, awtd);
  if (store)
    *store= cell;
  watched_cells->add(wc);
  // announce
  //uc->sim->mem_cell_changed(mem, addr);
  return(cell);
}

class cl_memory_cell *
cl_hw::use_cell(class cl_address_space *mem, t_addr addr,
                class cl_memory_cell **store,
                enum what_to_do_on_cell_change awtd)
{
  class cl_watched_cell *wc;
  class cl_memory_cell *cell;

  wc= new cl_used_cell(mem, addr, &cell, awtd);
  if (store)
    *store= cell;
  watched_cells->add(wc);
  return(cell);
}

void
cl_hw::mem_cell_changed(class cl_address_space *mem, t_addr addr)
{
  int i;

  for (i= 0; i < watched_cells->count; i++)
    {
      class cl_watched_cell *wc=
        (class cl_watched_cell *)(watched_cells->at(i));
      wc->mem_cell_changed(mem, addr, this);
    }
}

void
cl_hw::address_space_added(class cl_address_space *as)
{
  int i;

  for (i= 0; i < watched_cells->count; i++)
    {
      class cl_watched_cell *wc=
        dynamic_cast<class cl_watched_cell *>(watched_cells->object_at(i));
      wc->address_space_added(as, this);
    }
}


/*
 * Simulating `cycles' number of machine cycle
 */

int
cl_hw::tick(int cycles)
{
  return(0);
}

void
cl_hw::inform_partners(enum hw_event he, void *params)
{
  int i;

  for (i= 0; i < partners->count; i++)
    {
      class cl_partner_hw *ph= (class cl_partner_hw *)(partners->at(i));
      ph->happen(this, he, params);
    }
}


void
cl_hw::print_info(class cl_console_base *con)
{
  con->dd_printf("%s[%d]\n", id_string, id);
}


t_index
cl_hws::add(void *item)
{
  int i;
  t_index res;

  // pre-add
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->new_hw_adding((class cl_hw *)item);
    }
  // add
  res= cl_list::add(item);
  // post-add
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->new_hw_added((class cl_hw *)item);
    }
  ((class cl_hw *)item)->added_to_uc();
  return(res);
}


void
cl_hws::mem_cell_changed(class cl_address_space *mem, t_addr addr)
{
  int i;
  
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->mem_cell_changed(mem, addr);
    }
}

void
cl_hws::address_space_added(class cl_address_space *mem)
{
  int i;
  
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->address_space_added(mem);
    }
}


/*
 *____________________________________________________________________________
 */

cl_partner_hw::cl_partner_hw(class cl_uc *auc, enum hw_cath cath, int aid):
  cl_base()
{
  uc= auc;
  cathegory= cath;
  id= aid;
  partner= uc->get_hw(cathegory, id, 0);
}

class cl_hw *
cl_partner_hw::get_partner(void)
{
  return(partner);
}

void
cl_partner_hw::refresh(void)
{
  class cl_hw *hw= uc->get_hw(cathegory, id, 0);

  if (!hw)
    return;
  if (partner)
    {
      // partner is already set
      if (partner != hw)
        {
          // partner changed?
          partner= hw;
        }
      else
        partner= hw;
    }
  partner= hw;
}

void
cl_partner_hw::refresh(class cl_hw *new_hw)
{
  if (!new_hw)
    return;
  if (cathegory == new_hw->cathegory &&
      id == new_hw->id)
    {
      if (partner)
        {
          // partner changed?
          partner= new_hw;
        }
      else
        partner= new_hw;
    }
}

void
cl_partner_hw::happen(class cl_hw *where, enum hw_event he, void *params)
{
  if (partner)
    partner->happen(where, he, params);
}


/* End of hw.cc */
