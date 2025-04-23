/*
 * Simulator of microcontrollers (brkcl.h)
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

#ifndef SIM_BRKCL_HEADER
#define SIM_BRKCL_HEADER

#include "ddconfig.h"

// prj
#include "pobjcl.h"
#include "stypes.h"

// sim
#include "memcl.h"


/*
 * Base object of breakpoints
 */

class cl_brk: public cl_base
{
protected:
  class cl_address_space *mem;
  class cl_memory_cell *cell;
public:
  int nr;
  t_addr addr;
  enum  brk_perm perm;  // permanency (FIX,DYNAMIC)
  int   hit;
  int   cnt;
  chars cond;
  chars commands;
  
  cl_brk(class cl_address_space *imem, int inr, t_addr iaddr,
	 enum brk_perm iperm, int ihit);
  cl_brk(class cl_memory_cell *icell, int inr,
	 enum brk_perm iperm, int ihit);
  virtual ~cl_brk(void);

  class cl_address_space *get_mem(void) { return(mem); }
  class cl_memory_cell *get_cell(void);
  
  virtual bool condition(void);
  virtual void activate(void);
  virtual void inactivate(void);
  virtual enum brk_type type(void)= 0;
  virtual enum brk_event get_event(void)= 0;
  virtual bool do_hit(void);
  virtual void breaking(void);
};


/*
 * FETCH type of breakpoints
 */

class cl_fetch_brk: public cl_brk
{
public:
  uchar code;

  cl_fetch_brk(class cl_address_space *imem, int inr, t_addr iaddr,
	       enum brk_perm iperm, int ihit);

  virtual enum brk_type type(void);
  virtual enum brk_event get_event(void) { return(brkNONE); }
  virtual void breaking(void);
};


/*
 * Base of EVENT type of breakpoints
 */

class cl_ev_brk: public cl_brk
{
public:
  cl_ev_brk(class cl_address_space *imem,
	    int inr,
	    t_addr iaddr,
	    enum brk_perm iperm,
	    int ihit,
	    enum brk_event ievent,
	    const char *iid);
  cl_ev_brk(class cl_address_space *imem,
	    int inr,
	    t_addr iaddr,
	    enum brk_perm iperm,
	    int ihit,
	    char op);
  cl_ev_brk(class cl_memory_cell *icell,
	    int inr,
	    enum brk_perm iperm,
	    int ihit,
	    enum brk_event ievent,
	    const char *iid);
  cl_ev_brk(class cl_memory_cell *icell,
	    int inr,
	    enum brk_perm iperm,
	    int ihit,
	    char op);
  enum brk_event event;
  const char *id;

  virtual enum brk_type type(void);
  virtual enum brk_event get_event(void) { return(event); }
  virtual bool match(struct event_rec *ev);
  virtual void breaking(void);
};


/*
 * Collection of breakpoint sorted by address
 */

class brk_coll: public cl_sorted_list
{
public:
  class cl_address_space/*rom*/ *rom;
public:
  brk_coll(t_index alimit, t_index adelta, class cl_address_space/*rom*/*arom);
  virtual const void *key_of(const void *item) const;
  virtual int  compare(const void *key1, const void *key2);

  virtual bool there_is_event(enum brk_event ev);
  //virtual int make_new_nr(void);

  virtual void add_bp(class cl_brk *bp);
  virtual void del_bp(t_addr addr);
  virtual void del_bp(t_index idx, int /*dummy*/);
  virtual class cl_brk *get_bp(t_addr addr, int *idx);
  virtual class cl_brk *get_bp(int nr);
  virtual bool bp_at(t_addr addr);
};

class cl_display: public chars
{
public:
  int nr;
  chars fmt;
public:
  cl_display(const chars &f, const chars &e): chars(e) { nr=0; fmt= f; }
};

class cl_display_list: public cl_list
{
protected:
  int cnt;
public:
  cl_display_list(void): cl_list()
  {
    cnt= 0;
  }
  cl_display_list(t_index alimit, t_index adelta, const char *aname):
    cl_list(alimit, adelta, aname)
  {
    cnt= 0;
  }
public:
  virtual t_index  add(void *item);
  virtual void undisplay(int nr);
  virtual void do_display(class cl_console_base *con);
};


#endif

/* End of sim.src/brkcl.h */
