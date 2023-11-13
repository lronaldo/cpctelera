/*
 * Simulator of microcontrollers (sim.src/argcl.h)
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

#ifndef SIM_ARGCL_HEADER
#define SIM_ARGCL_HEADER

// prj
#include "pobjcl.h"
#include "stypes.h"


/*
 * Base type of arguments/parameters
 */

class cl_arg: public cl_base
{
public:
  union {
    long i_value;
    double f_value;
    void *p_value;
  };
  char *s_value;

public:  
  cl_arg(long lv);
  cl_arg(const char *lv);
  cl_arg(double fv);
  cl_arg(void *pv);
  virtual ~cl_arg(void);

  virtual bool get_ivalue(long *value);
  virtual char *get_svalue(void);
  virtual double get_fvalue(void);
  virtual void *get_pvalue(void);
  virtual bool get_bit_address(class cl_uc *uc, // input
			       class cl_address_space **mem, // outputs
			       t_addr *mem_addr,
			       t_mem *bit_mask) { return(false); }
};


/*
 * Command parameters
 */

class cl_cmd_arg: public cl_arg
{
public:
  //class cl_uc *uc;
  bool interpreted_as_string;
  union {
    long number;
    t_addr address;
    t_mem data;
    struct {
      class cl_memory *memory;
      class cl_address_space *address_space;
      class cl_memory_chip *memchip;
    } memory;
    class cl_hw *hw;
    struct {
      int len;
      char *string;
    } string;
    struct {
      t_mem *array;
      int len;
    } data_list;
    struct {
      class cl_address_space *mem;
      t_addr mem_address;
      t_mem mask;
    } bit;
    class cl_memory_cell *cell;
  } value;

public:
  cl_cmd_arg(long i): cl_arg(i)
  { interpreted_as_string= false; }
  cl_cmd_arg(const char *s): cl_arg(s)
  { interpreted_as_string= false; }
  virtual ~cl_cmd_arg(void);
  virtual int init(void);
  
  virtual int is_string(void) { return(false); }
  virtual bool get_address(class cl_uc *uc, t_addr *addr) { return(false); }
  virtual bool as_address(class cl_uc *uc);
  virtual bool as_number(void);
  virtual bool as_data(void);
  virtual bool as_string(void);
  virtual bool as_memory(class cl_uc *uc);
  virtual bool as_hw(class cl_uc *uc);
  virtual bool as_cell(class cl_uc *uc);
  virtual bool as_bit(class cl_uc *uc);
};

class cl_cmd_int_arg: public cl_cmd_arg
{
public:
  cl_cmd_int_arg(long addr);

  virtual bool get_address(class cl_uc *uc, t_addr *addr);
  virtual bool get_bit_address(class cl_uc *uc, // input
			       class cl_address_space **mem, // outputs
			       t_addr *mem_addr,
			       t_mem *bit_mask);
  virtual bool as_string(void);
};

class cl_cmd_sym_arg: public cl_cmd_arg
{
public:
  cl_cmd_sym_arg(const char *sym);

  virtual bool get_address(class cl_uc *uc, t_addr *addr);
  virtual bool get_bit_address(class cl_uc *uc, // input
			       class cl_address_space **mem, // outputs
			       t_addr *mem_addr,
			       t_mem *bit_mask);
  virtual bool as_address(class cl_uc *uc);
  virtual bool as_number(void) { return(false); }
  virtual bool as_string(void);
  virtual bool as_hw(class cl_uc *uc);
  virtual bool as_cell(class cl_uc *uc);
};

class cl_cmd_str_arg: public cl_cmd_arg
{
public:
  cl_cmd_str_arg(const char *str);

  virtual int is_string(void) { return(1); }
  virtual bool as_number(void) { return(false); }
};

class cl_cmd_bit_arg: public cl_cmd_arg
{
public:
  class cl_cmd_arg *sfr, *bit;

public:
  cl_cmd_bit_arg(class cl_cmd_arg *asfr, class cl_cmd_arg *abit);
  virtual ~cl_cmd_bit_arg(void);

  virtual bool get_address(class cl_uc *uc, t_addr *addr);
  virtual bool get_bit_address(class cl_uc *uc, // input
			       class cl_address_space **mem, // outputs
			       t_addr *mem_addr,
			       t_mem *bit_mask);
};

class cl_cmd_array_arg: public cl_cmd_arg
{
public:
  class cl_cmd_arg *name_arg, *index;

public:
  cl_cmd_array_arg(class cl_cmd_arg *aname, class cl_cmd_arg *aindex);
  virtual ~cl_cmd_array_arg(void);
  virtual bool as_hw(class cl_uc *uc);
  virtual bool as_cell(class cl_uc *uc);
};


/*
 * Program arguments
 */

/*class cl_prg_arg: public cl_arg
{
public:
  char short_name;
  char *long_name;

public:
  cl_prg_arg(char sn, char *ln, long lv);
  cl_prg_arg(char sn, char *ln, char *lv);
  cl_prg_arg(char sn, char *ln, double fv);
  cl_prg_arg(char sn, char *ln, void *pv);
  virtual ~cl_prg_arg(void);
};*/


/*
 * List of arguments
 */

/*class cl_arguments: public cl_list
{
public:
  cl_arguments(void): cl_list(5, 5) {}

  int arg_avail(char nam);
  int arg_avail(char *nam);
  virtual long get_iarg(char sname, char *lname);
  virtual char *get_sarg(char sname, char *lname);
  virtual double get_farg(char sname, char *lname);
  virtual void *get_parg(char sname, char *lname);
};*/


#endif

/* End of argcl.h */
