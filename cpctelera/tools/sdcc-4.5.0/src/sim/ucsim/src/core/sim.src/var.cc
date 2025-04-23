/*
 * Simulator of microcontrollers (var.cc)
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

#include <string.h>
#include <ctype.h>

#include "varcl.h"


/* Cell only variable */

cl_cvar::cl_cvar(chars iname, class cl_memory_cell *icell, chars adesc, int ibitnr_high, int ibitnr_low):
  cl_base()
{
  if (ibitnr_low < ibitnr_high)
    {
      bitnr_low= ibitnr_low;
      bitnr_high= ibitnr_high;
    }
  else
    {
      bitnr_low= ibitnr_high;
      bitnr_high= ibitnr_low;
    }
  desc= adesc;
  
  set_name(iname);
  defined_by= VBY_PRE;
  
  cell= icell;
}

int
cl_cvar::init(void)
{
  return 0;
}

t_mem
cl_cvar::write(t_mem val)
{
  if (!cell)
    return 0;
  return cell->write(val);
}

t_mem
cl_cvar::read()
{
  if (!cell)
    return 0;
  return cell->read();
}

t_mem
cl_cvar::set(t_mem val)
{
  if (!cell)
    return 0;
  return cell->set(val);
}

void
cl_cvar::print_info(cl_console_base *con) const
{
  con->dd_printf("%s ", get_name("?"));
  t_mem m= cell->/*get*/read();
  if (bitnr_high >= 0)
    {
      if (bitnr_high != bitnr_low)
        {
          con->dd_printf("[%u:%u] = 0b", bitnr_high, bitnr_low);
          for (int i= bitnr_high; i >= bitnr_low; i--)
            con->dd_printf("%c", (m & (1U << i)) ? '1' : '0');
          m &= ((1U << (bitnr_high - bitnr_low + 1)) - 1) << bitnr_low;
        }
      else
        {
          m &= (1U << bitnr_low);
          con->dd_printf(".%u = %c", bitnr_low, m ? '1' : '0');
        }
    }
  else
    {
      con->dd_printf(" = 0x");
      con->dd_printf(/*mem->data_format*/"%08x", m);
    }
  con->dd_printf(",%uU", MU(m));
  con->dd_printf(",%d", MI(m));
  if ((MU(m) < 0x100) && isprint(MI(m)))
    con->dd_printf(",'%c'", MI(m));
  con->dd_printf("\n");
  if (desc.nempty())
    con->dd_printf("  %s\n", desc.c_str());
}


/* Variable pointing to memory location */

cl_var::cl_var(chars iname, class cl_memory *imem, t_addr iaddr, chars adesc, int ibitnr_high, int ibitnr_low):
  cl_cvar(iname, NULL, adesc, ibitnr_high, ibitnr_low)
{
  mem= imem;
  addr= iaddr;
}

int
cl_var::init(void)
{
  if (!mem ||
      !mem->is_address_space() ||
      !mem->valid_address(addr))
    return 0;
  cell= static_cast<cl_address_space *>(mem)->get_cell(addr);
  return 0;
}

void
cl_var::print_info(cl_console_base *con) const
{
  con->dd_printf("%s ", get_name("?"));
  con->dd_printf("%s", mem->get_name("?"));
  con->dd_printf("[");
  con->dd_printf(mem->addr_format, addr);
  con->dd_printf("]");
  t_mem m= mem->read(addr);
  if (bitnr_high >= 0)
    {
      if (bitnr_high != bitnr_low)
        {
          con->dd_printf("[%u:%u] = 0b", bitnr_high, bitnr_low);
          for (int i= bitnr_high; i >= bitnr_low; i--)
            con->dd_printf("%c", (m & (1U << i)) ? '1' : '0');
          m &= ((1U << (bitnr_high - bitnr_low + 1)) - 1) << bitnr_low;
        }
      else
        {
          m &= (1U << bitnr_low);
          con->dd_printf(".%u = %c", bitnr_low, m ? '1' : '0');
        }
    }
  else
    {
      con->dd_printf(" = 0x");
      con->dd_printf(mem->data_format, m);
    }
  con->dd_printf(",%uU", MU(m));
  con->dd_printf(",%d", MI(m));
  if ((MU(m) < 0x100) && isprint(MI(m)))
    con->dd_printf(",'%c'", MI(m));
  con->dd_printf("\n");
  if (desc.nempty())
    con->dd_printf("  %s\n", desc.c_str());
}


cl_var_by_name_list::~cl_var_by_name_list(void)
{
}

const void *
cl_var_by_name_list::key_of(const void *item) const
{
  return static_cast<const class cl_var *>(item)->get_name();
}

int
cl_var_by_name_list::compare(const void *key1, const void *key2)
{
  if (key1 && key2)
    return strcmp(static_cast<const char *>(key1), static_cast<const char *>(key2));
  return 0;
}


cl_var_by_addr_list::~cl_var_by_addr_list(void)
{
}

int
cl_var_by_addr_list::compare_addr(class cl_var *var, class cl_memory *mem, t_addr addr)
{
  int ret;
  class cl_memory *vmem= var->get_mem();
  t_addr vaddr= var->get_addr();
  if (!vmem)
    return 0;
  
  if (!(ret = vmem->get_uid() - mem->get_uid()))
    ret = vaddr - addr;

  return ret;
}

int
cl_var_by_addr_list::compare_addr_and_bits(class cl_var *var, class cl_memory *mem, t_addr addr, int bitnr_high, int bitnr_low)
{
  int ret;
  class cl_memory *vmem= var->get_mem();
  t_addr vaddr= var->get_addr();
  if (!vmem)
    return 0;
  
  if (!(ret = vmem->get_uid() - mem->get_uid()) &&
     !(ret = vaddr - addr) &&
     (!(ret = (var->bitnr_high < 0
               ? (bitnr_high < 0 ? 0 : -1)
               : (bitnr_high < 0
                  ? 1
                  : bitnr_high - var->bitnr_high)))))
    ret = (var->bitnr_low < 0
           ? (bitnr_low < 0 ? 0 : -1)
           : (bitnr_low < 0
              ? 1
              : var->bitnr_low - bitnr_low));

  return ret;
}

int
cl_var_by_addr_list::compare(const void *key1, const void *key2)
{
  class cl_var *k1 = (cl_var*)(key1);
  class cl_var *k2 = (cl_var*)(key2);
  int ret;

  // An addr may have multiple names as long as they are all different.
  if (!(ret = compare_addr_and_bits(k1, k2->get_mem(), k2->get_addr(), k2->bitnr_high, k2->bitnr_low)))
    ret = strcmp(k1->get_name(), k2->get_name());

  return ret;
}

bool
cl_var_by_addr_list::search(class cl_memory *mem, t_addr addr, t_index &index)
{
  t_index l  = 0;
  t_index h  = count - 1;
  bool    res= false;

  if (!mem)
    return false;
  while (l <= h)
    {
      t_index i= (l + h) >> 1;
      t_index c= compare_addr((cl_var *)(key_of(Items[i])), mem, addr);
      if (c < 0) l= i + 1;
      else
        {
          h= i - 1;
          if (c == 0)
            {
              res= true;
              // We want the _first_ name for the given addr.
              for (l = i; l > 0 && !compare_addr((cl_var *)(key_of(Items[l-1])), mem, addr); l--);
            }
        }
    }

  index= l;
  return(res);
}

bool
cl_var_by_addr_list::search(class cl_memory *mem, t_addr addr, int bitnr_high, int bitnr_low, t_index &index)
{
  t_index l  = 0;
  t_index h  = count - 1;
  bool    res= false;

  if (!mem)
    return false;
  while (l <= h)
    {
      t_index i= (l + h) >> 1;
      t_index c= compare_addr_and_bits((cl_var *)(key_of(Items[i])), mem, addr, bitnr_high, bitnr_low);
      if (c < 0) l= i + 1;
      else
        {
          h= i - 1;
          if (c == 0)
            {
              res= true;
              // We want the _first_ name for the given addr.
              for (l = i; l > 0 && !compare_addr_and_bits((cl_var *)(key_of(Items[l-1])), mem, addr, bitnr_high, bitnr_low); l--);
            }
        }
    }

  index= l;
  return(res);
}

size_t
cl_var_list::get_max_name_len(void)
{
  if (!max_name_len_known)
    {
      max_name_len = 0;

      for (t_index i = 0; i < by_name.count; i++)
        {
          size_t l = strlen(by_name.at(i)->get_name());
          if (l > max_name_len)
            max_name_len = l;
        }

      max_name_len_known = true;
    }

  return max_name_len;
}

bool
cl_var_list::del(const char *name)
{
  t_index name_i;
  bool found = by_name.search(name, name_i);

  if (found)
    {
      cl_cvar *var = by_name.at(name_i);

      by_addr.disconn(var);
      by_name.disconn_at(name_i);
      delete var;

      max_name_len_known = false;
    }

  return found;
}

class cl_cvar *
cl_var_list::add(class cl_cvar *item)
{
  const char *name = item->get_name();

  if (!del(name))
    {
      size_t l = strlen(name);
      if (l > max_name_len)
        max_name_len = l;
    }
  else
    {
      // We're replacing with the same name so we still know the max length really
      max_name_len_known = true;
    }

  by_name.add(item);
  if (item->is_mem_var())
    {
      class cl_var *var = static_cast<class cl_var *>(item);

      // If analyze put a variable at this location in the past it is no
      // longer needed and can be removed.
      t_index i;
      if (by_addr.search(var->get_mem(), var->get_addr(), var->bitnr_high, var->bitnr_low, i))
        {
          class cl_var *v= 0;
          while (i < by_addr.count && (v = by_addr.at(i)) &&
                 v->get_mem() == var->get_mem() && v->get_addr() == var->get_addr() &&
                 v->bitnr_high == var->bitnr_high && v->bitnr_low == var->bitnr_low)
            {
              if (*(v->get_name()) == '.')
                {
                  by_addr.disconn_at(i);
                  by_name.disconn(v);

                  if (strlen(v->get_name()) == max_name_len &&
                      strlen(item->get_name()) < max_name_len)
                    max_name_len_known = false;

                  delete v;

                  // There should be at most 1.
                  break;
                }

                i++;
            }
        }

      by_addr.add(item);
    }

  return item;
}

class cl_cvar *
cl_var_list::add(chars name, class cl_memory *mem, t_addr addr, int bitnr_high, int bitnr_low, chars desc)
{
  class cl_cvar *var;

  var = new cl_var(name, mem, addr, desc, bitnr_high, bitnr_low);
  var->init();
  return add(var);
}

class cl_cvar *
cl_var_list::add(chars name, const char *cellname, int bitnr_high, int bitnr_low, chars desc)
{
  int i;
  if (by_name.search(cellname, i))
    {
      class cl_cvar *var = (cl_var*)by_name.at(i);
      return add(name, var->get_mem(), var->get_addr(), bitnr_high, bitnr_low, desc);
    }

  return NULL;
}

void
cl_var_list::add(chars prefix, class cl_memory *mem, t_addr base, const struct var_def *def, size_t n)
{
  chars regname = chars("");
  /*t_addr*/i64_t offset = 0;

  for (; n; def++, n--)
    {
      class cl_var *var;

      if (def->bitnr_high < 0)
        {
          regname = prefix + def->name;

          if (def->bitnr_low < 0)
            {
              offset = -1;
              int i;
              if (by_name.search(regname, i))
                {
                  cl_var *var = (cl_var*)by_name.at(i);
                  if (var->get_mem() == mem)
                    offset = var->get_addr() - base;
                }
            }
          else
            {
              offset = def->bitnr_low;

              var = new cl_var(regname, mem, base + offset, def->desc, -1, -1);
              var->init();
              add(var);

              regname += "_";
            }
        }
      else if (mem && offset >= 0)
        {
          var = new cl_var(regname + def->name, mem, base + offset, def->desc, def->bitnr_high, def->bitnr_low);
          var->init();
          add(var);
        }
    }
}

class cl_var *
cl_var_list::by_cell(class cl_memory_cell *c)
{
  t_index i;
  for (i= 0; i<by_name.count; i++)
    {
      class cl_var *v= (class cl_var *)(by_name.at(i));
      class cl_memory_cell *cell= v->get_cell();
      if (cell == c)
	return v;
    }
  return NULL;
}

t_mem
cl_var_list::read(chars name)
{
  bool found;
  t_index i;
  class cl_cvar *v;
  found= by_name.search(name, i);
  if (found)
    {
      v= by_name.at(i);
      return v->read();
    }
  return 0;
}

int
cl_vars_iterator::compare_bits(const class cl_var *var1, const class cl_var *var2)
{
  int ret;

  if ((!(ret = (var1->bitnr_high < 0
               ? (var2->bitnr_high < 0 ? 0 : -1)
               : (var2->bitnr_high < 0
                  ? 1
                  : var2->bitnr_high - var1->bitnr_high)))))
    ret = (var1->bitnr_low < 0
           ? (var2->bitnr_low < 0 ? 0 : -1)
           : (var2->bitnr_low < 0
              ? 1
              : var1->bitnr_low - var2->bitnr_low));

  return ret;
}

const cl_var *
cl_vars_iterator::first(cl_memory *mem, t_addr addr)
{
  const cl_var *space_var = NULL;
  space_mem = mem;
  space_addr = addr;


  if (vars->by_addr.search(space_mem, space_addr, space_i))
    space_var = vars->by_addr.at(space_i);

  const cl_var *chip_var = NULL;
  chip_mem = NULL;

  cl_address_decoder *ad= 0;
  if (space_mem->is_address_space() &&
      (ad = ((cl_address_space *)space_mem)->get_decoder_of(space_addr)))
    {
      chip_mem = ad->memchip;
      chip_addr = ad->as_to_chip(space_addr);

      if (vars->by_addr.search(chip_mem, chip_addr, chip_i))
        chip_var = vars->by_addr.at(chip_i);
    }

  if (chip_var && (!space_var || compare_bits(chip_var, space_var) < 0))
    {
      chip_i++;
      return chip_var;
    }

  space_i++;
  return space_var;
}

const cl_var *
cl_vars_iterator::next(void)
{
  const cl_var *space_var = NULL;
  const cl_var *chip_var = NULL;

  if (space_i >= 0 && space_i < vars->by_addr.count)
    {
      space_var = vars->by_addr.at(space_i);
      if (space_var->get_mem() != space_mem || space_var->get_addr() != space_addr)
        {
          space_i = -1;
          space_var = NULL;
        }
    }

  if (chip_i >= 0 && chip_i < vars->by_addr.count)
    {
      chip_var = vars->by_addr.at(chip_i);
      if (chip_var->get_mem() != chip_mem || chip_var->get_addr() != chip_addr)
        {
          chip_i = -1;
          chip_var = NULL;
        }
    }

  if (chip_var && (!space_var || compare_bits(chip_var, space_var) < 0))
    {
      chip_i++;
      return chip_var;
    }

  if (space_var)
    {
      space_i++;
      return space_var;
    }

  return NULL;
}

/* End of sim.src/var.cc */
