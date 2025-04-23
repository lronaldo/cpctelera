/*
 * Simulator of microcontrollers (varcl.h)
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

#ifndef SIM_VARCL_HEADER
#define SIM_VARCL_HEADER


#include "pobjcl.h"

#include "newcmdcl.h"

#include "memcl.h"


enum var_by {
  VBY_PRE	= 'p',
  VBY_USER	= 'u',
  VBY_DEBUG	= 'd',
  VBY_ANALYZE	= 'a'
};

class cl_cvar: public cl_base
{
 public:
  int bitnr_high, bitnr_low;
  chars desc;
  enum var_by defined_by;
 protected:
  class cl_memory_cell *cell;
 public:
  cl_cvar(chars iname, class cl_memory_cell *icell, chars adesc, int ibitnr_high= -1, int ibitnr_low= -1);
  virtual int init(void);
  virtual void set_by(enum var_by by) { defined_by= by; }
  virtual class cl_memory_cell *get_cell(void) const { return cell; }
  virtual class cl_memory *get_mem() const { return NULL; }
  virtual t_addr get_addr() const { return 0; }
  virtual bool is_mem_var() const { return false; }
  virtual void set_cell(cl_memory_cell *new_cell) { cell= new_cell; }
  
  virtual t_mem write(t_mem val);
  virtual t_mem read();
  virtual t_mem set(t_mem val);

  virtual void print_info(cl_console_base *con) const;
};

class cl_var: public cl_cvar
{
protected:
  class cl_memory *mem;
  t_addr addr;
public:
  cl_var(chars iname, class cl_memory *imem, t_addr iaddr, chars adesc, int ibitnr_high= -1, int ibitnr_low= -1);
  virtual int init(void);
  virtual class cl_memory *get_mem() const { return mem; }
  virtual t_addr get_addr() const { return addr; }
  virtual bool is_mem_var() const { return true; }

  virtual void print_info(cl_console_base *con) const;
};


class cl_var_by_name_list: public cl_sorted_list
{
 public:
  cl_var_by_name_list(): cl_sorted_list(10, 10, "symlist") {}
  ~cl_var_by_name_list(void);

  class cl_cvar *at(t_index index)
  {
    const void *a= cl_sorted_list::at(index);
    class cl_cvar *v= (class cl_cvar *)a;
    return v;
  }

 private:
  virtual const void *key_of(const void *item) const;
  virtual int compare(const void *key1, const void *key2);
};

class cl_var_by_addr_list: public cl_sorted_list
{
 public:
  cl_var_by_addr_list(): cl_sorted_list(10, 10, "symlist_by_addr") {}
  ~cl_var_by_addr_list(void);

  class cl_var *at(t_index index)
  {
    const void *a= cl_sorted_list::at(index);
    class cl_var *v= (class cl_var *)a;
    return v;
  }
  bool search(class cl_memory *mem, t_addr addr, t_index &index);
  bool search(class cl_memory *mem, t_addr addr, int bitnr_high, int bitnr_low, t_index &index);

  int compare(const void *key1, const void *key2);

 private:
  int compare_addr(class cl_var *var, class cl_memory *mem, t_addr addr);
  int compare_addr_and_bits(class cl_var *var, class cl_memory *mem, t_addr addr, int bitnr_high, int bitnr_low);
};

struct var_def {
  const char *name;
  int bitnr_high, bitnr_low;
  const char *desc;
};

#define var_thiscell()	-1, -1
#define var_offset(N)	-1, N
#define var_bit(N)	N, N
#define var_bitset(H,L)	H, L

class cl_var_list: public cl_base
{
 private:
  size_t max_name_len;
  bool max_name_len_known;

 public:
  class cl_var_by_name_list by_name;
  class cl_var_by_addr_list by_addr;

 public:
  cl_var_list() { max_name_len = 0; max_name_len_known = true; }

  /*! \brief Add the given cl_var replacing any that already exist with the same name.
   */
  class cl_cvar *add(class cl_cvar *item);

  /*! \brief Create and add (or replace) a var naming a set of bits in the cell given by mem and addr.
   */
  class cl_cvar *add(chars name, class cl_memory *mem, t_addr addr, int bitnr_high, int bitnr_low, chars desc);

  /*! \brief Create and add (or replace) a var naming a set of bits in the cell given by a named var.
   */
  class cl_cvar *add(chars name, const char *cellname, int bitnr_high, int bitnr_low, chars desc);

  /*! \brief Create and add (or replace) a var labeling a cell.
   */
  class cl_cvar *add(chars name, class cl_memory *mem, t_addr addr, chars desc) {
    return add(name, mem, addr, -1, -1, desc);
  }

  /*! \brief Create and add (or replace) vars using the given list of definitions.
   */
  void add(chars prefix, class cl_memory *mem, t_addr base, const struct var_def *def, size_t n);

  /*! \brief Delete the var with the given name.
   */
  bool del(const char *name);

  virtual cl_var *by_cell(class cl_memory_cell *c);
  
  /*! \brief Return the length of the longest var name.
   */
  size_t get_max_name_len(void);

  virtual t_mem read(chars name);
};

class cl_vars_iterator {
  private:
    cl_var_list *vars;
    cl_memory *space_mem, *chip_mem;
    t_addr space_addr, chip_addr;
    t_index space_i, chip_i;

  public:
    cl_vars_iterator(cl_var_list *vars) { this->vars = vars; };

    const cl_var *first(cl_memory *mem, t_addr addr);
    const cl_var *next(void);

  private:
    int compare_bits(const class cl_var *var1, const class cl_var *var2);
};

#endif

/* End of sim.src/varcl.h */
