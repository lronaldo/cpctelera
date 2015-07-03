/*
 * Simulator of microcontrollers (sim.src/hwcl.h)
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

/* Abstract hw element. It can be a timer, serial line or whatever */

#ifndef SIM_HWCL_HEADER
#define SIM_HWCL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "guiobjcl.h"

// cmd.src
#include "newcmdcl.h"

// local
#include "memcl.h"
#include "uccl.h"


enum what_to_do_on_cell_change {
  wtd_none              = 0x01,
  wtd_write             = 0x02,
  wtd_restore           = 0x04,
  wtd_restore_write     = 0x08
};

#define WTD_WRITE       (wtd_write|wtd_restore_write)
#define WTD_RESTORE     (wtd_restore|wtd_restore_write)

class cl_hw; // forward

class cl_watched_cell: public cl_base
{
protected:
  class cl_address_space *mem;
  t_addr addr;
  class cl_memory_cell *cell;
  class cl_memory_cell **store;
public:
  enum what_to_do_on_cell_change wtd;
public:
  cl_watched_cell(class cl_address_space *amem, t_addr aaddr,
                  class cl_memory_cell **astore,
                  enum what_to_do_on_cell_change awtd);

  virtual void mem_cell_changed(class cl_address_space *amem, t_addr aaddr,
                                class cl_hw *hw);
  virtual void address_space_added(class cl_address_space *amem,
                                   class cl_hw *hw);
};

class cl_used_cell: public cl_watched_cell
{
public:
  cl_used_cell(class cl_address_space *amem, t_addr aaddr,
               class cl_memory_cell **astore,
               enum what_to_do_on_cell_change awtd):
    cl_watched_cell(amem, aaddr, astore, awtd) {}

  virtual void mem_cell_changed(class cl_address_space *amem, t_addr aaddr,
                                class cl_hw *hw);
  virtual void address_space_added(class cl_address_space *amem,
                                   class cl_hw *hw);
};

class cl_hw: public cl_guiobj
{
public:
  int flags;
  class cl_uc *uc;
  enum hw_cath cathegory;
  int id;
  char *id_string;
protected:
  class cl_list *partners;
  class cl_list *watched_cells;
public:
  cl_hw(class cl_uc *auc, enum hw_cath cath, int aid, const char *aid_string);
  virtual ~cl_hw(void);

  virtual void new_hw_adding(class cl_hw *new_hw);
  virtual void new_hw_added(class cl_hw *new_hw);
  virtual void added_to_uc(void) {}
  virtual class cl_hw *make_partner(enum hw_cath cath, int id);

  virtual t_mem read(class cl_memory_cell *cell) { return(cell->get()); }
  virtual void write(class cl_memory_cell * /*cell*/, t_mem * /*val*/) {}

  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual class cl_memory_cell *register_cell(class cl_address_space *mem,
                                              t_addr addr,
                                              class cl_memory_cell **store,
                                              enum what_to_do_on_cell_change
                                              awtd);
  virtual class cl_memory_cell *use_cell(class cl_address_space *mem,
                                         t_addr addr,
                                         class cl_memory_cell **store,
                                         enum what_to_do_on_cell_change awtd);
  virtual void mem_cell_changed(class cl_address_space *mem, t_addr addr);
  virtual void address_space_added(class cl_address_space *as);

  virtual int tick(int cycles);
  virtual void reset(void) {}
  virtual void happen(class cl_hw * /*where*/, enum hw_event /*he*/,
                      void * /*params*/) {}
  virtual void inform_partners(enum hw_event he, void *params);

  virtual void print_info(class cl_console_base *con);
};

class cl_hws: public cl_list
{
public:
  cl_hws(void): cl_list(2, 2, "hws") {}
  virtual t_index add(void *item);
  virtual void mem_cell_changed(class cl_address_space *mem, t_addr addr);
  virtual void address_space_added(class cl_address_space *as);
};


class cl_partner_hw: public cl_base
{
protected:
  class cl_uc *uc;
  enum hw_cath cathegory;
  int id;
  class cl_hw *partner;
public:
  cl_partner_hw(class cl_uc *auc, enum hw_cath cath, int aid);

  virtual class cl_hw *get_partner(void);
  virtual void refresh(void);
  virtual void refresh(class cl_hw *new_hw);

  virtual void happen(class cl_hw *where, enum hw_event he, void *params);
};


#endif

/* End of hwcl.h */
