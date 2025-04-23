/*
 * Simulator of microcontrollers (arg.cc)
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
#include <stdlib.h>
#include <string.h>
//#include "i_string.h"

// prj
#include "globals.h"

// sim
//#include "simcl.h"

// cmd
//#include "cmdutil.h"

// local
#include "argcl.h"


/*
 * Making the argument
 */

cl_arg::cl_arg(long lv):
  cl_base()
{
  i_value= lv;
  s_value= 0;
}

cl_arg::cl_arg(const char *sv):
  cl_base()
{
  s_value= sv?strdup(sv):0;
}

cl_arg::cl_arg(double fv):
  cl_base()
{
  f_value= fv;
  s_value= 0;
}

cl_arg::cl_arg(void *pv):
  cl_base()
{
  p_value= pv;
  s_value= 0;
}

cl_arg::~cl_arg(void)
{
  if (s_value)
    free((void*)s_value);
}


/*
 * Getting value of the argument
 */

bool
cl_arg::get_ivalue(long *value)
{
  if (value)
    *value= i_value;
  return(true);
}

char *
cl_arg::get_svalue(void)
{
  return(s_value);
}

double
cl_arg::get_fvalue(void)
{
  return(f_value);
}

void *
cl_arg::get_pvalue(void)
{
  return(p_value);
}


/*
 * Command parameters
 *----------------------------------------------------------------------------
 */

cl_cmd_arg::~cl_cmd_arg(void)
{
  if (interpreted_as_string)
    {
      if (value.string.string)
	free(value.string.string);
    }
}

int
cl_cmd_arg::init(void)
{
  return 0;
}

bool
cl_cmd_arg::as_address(class cl_uc *uc)
{
  return(get_address(uc, &(value.address)));    
}

bool
cl_cmd_arg::as_number(void)
{
  return(get_ivalue(&(value.number)));
}

bool
cl_cmd_arg::as_data(void)
{
  long l;
  bool ret= get_ivalue(&l);
  value.data= l;
  return(ret);
}

bool
cl_cmd_arg::as_memory(class cl_uc *uc)
{
  value.memory.memory= uc->memory(get_svalue());
  value.memory.address_space= 0;
  value.memory.memchip= 0;
  if (value.memory.memory)
    {
      if (value.memory.memory->is_chip())
	value.memory.memchip=
	  (class cl_memory_chip *)(value.memory.memory);
      if (value.memory.memory->is_address_space())
	value.memory.address_space=
	  (class cl_address_space *)(value.memory.memory);
    }
  return(value.memory.memory != 0);
}

bool
cl_cmd_arg::as_hw(class cl_uc *uc)
{
  return(false);
}

bool
cl_cmd_arg::as_cell(class cl_uc *uc)
{
  return(false);
}

bool
cl_cmd_arg::as_string(void)
{
  char *s= get_svalue();
  if (!s)
    return(false);
  if (is_string())
    value.string.string= proc_escape(s, &value.string.len);
  else
    {
      value.string.string= strdup(s);
      value.string.len= strlen(s);
    }
  return(interpreted_as_string= value.string.string != NULL);
}

bool
cl_cmd_arg::as_bit(class cl_uc *uc)
{
  if (interpreted_as_string)
    {
      interpreted_as_string= false;
      if (value.string.string)
        free(value.string.string);
    }

  return(get_bit_address(uc,
                         &(value.bit.mem),
                         &(value.bit.mem_address),
                         &(value.bit.bitnr_high),
                         &(value.bit.bitnr_low)));
}


/* Integer number */

cl_cmd_int_arg::cl_cmd_int_arg(long addr):
  cl_cmd_arg(addr)
{}

bool
cl_cmd_int_arg::get_address(class cl_uc *uc, t_addr *addr)
{
  long iv;

  bool b= get_ivalue(&iv);
  if (addr)
    *addr= iv;
  return(b);
}

bool
cl_cmd_int_arg::get_bit_address(class cl_uc *uc, // input
                                class cl_memory **mem, // outputs
                                t_addr *mem_addr,
                                int *bitnr_high,
                                int *bitnr_low)
{
  t_addr bit_addr;

  if (!get_address(uc, &bit_addr))
    return(false);
  
  if (bitnr_high)
    *bitnr_high = -1;
  if (bitnr_low)
    *bitnr_low = -1;

  if (mem)
    if (!(*mem= uc->bit2mem(bit_addr, mem_addr, bitnr_high, bitnr_low)))
      *mem = uc->rom;

  return(true);
}

bool
cl_cmd_int_arg::as_string(void)
{
  value.string.string= (char*)malloc(100);
  sprintf(value.string.string, "%ld", i_value);
  value.string.len= strlen(value.string.string);
  return(interpreted_as_string= value.string.string != NULL);
}


/* Symbol */

cl_cmd_sym_arg::cl_cmd_sym_arg(const char *sym):
  cl_cmd_arg(sym)
{}

bool
cl_cmd_sym_arg::as_string(void)
{
  char *s= get_svalue();
  if (!s)
    return(false);
  value.string.string= strdup(s);
  value.string.len= strlen(s);
  return(interpreted_as_string= value.string.string != NULL);
}

bool
cl_cmd_sym_arg::get_address(class cl_uc *uc, t_addr *addr)
{
  t_addr a;

  if (uc->symbol2address(get_svalue(), NULL, &a))
    {
      if (addr)
	*addr= a;
      return 1;
    }
  return(0);
}

bool
cl_cmd_sym_arg::get_bit_address(class cl_uc *uc, // input
                                class cl_memory **mem, // outputs
                                t_addr *mem_addr,
                                int *bitnr_high,
                                int *bitnr_low)
{
  t_index i;
  if (uc->vars->by_name.search(get_svalue(), i))
    {
      class cl_var *v= (cl_var*)uc->vars->by_name.at(i);
      if (mem)
        *mem= v->get_mem();
      if (mem_addr)
        *mem_addr= v->get_addr();
      if (bitnr_high)
        *bitnr_high= v->bitnr_high;
      if (bitnr_low)
        *bitnr_low= v->bitnr_low;
      return true;
    }
  return false;
}

bool
cl_cmd_sym_arg::as_address(class cl_uc *uc)
{
  t_addr a;
  if (uc->symbol2address(get_svalue(), NULL, &a))
    {
      value.address= a;
      return true;
    }
  return(false);
}

bool
cl_cmd_sym_arg::as_hw(class cl_uc *uc)
{
  cl_hw *hw, *found;
  int i= 0;

  hw= found= uc->get_hw(get_svalue(), &i);
  if (!hw)
    return(false);
  if (hw && (strcmp(get_svalue(), "cpu")==0))
    return value.hw= hw, true;
  i++;
  found= uc->get_hw(get_svalue(), &i);
  if (found)
    return(false);
  value.hw= hw;
  return(true);
}

bool
cl_cmd_sym_arg::as_cell(class cl_uc *uc)
{
  class cl_memory *mem;
  class cl_memory_cell *cell;
  t_addr addr;
  
  if (uc->symbol2address(get_svalue(), &mem, &addr))
    {
      if (mem->is_address_space())
	{
	  value.cell= ((cl_address_space *)mem)->get_cell(addr);
	  return value.cell != NULL;
	}
      return false;
    }
  else if (uc->symbol2cell(get_svalue(), &cell))
    {
      value.cell= cell;
      return true;
    }
  return false;
}


/* String */

cl_cmd_str_arg::cl_cmd_str_arg(const char *str):
  cl_cmd_arg(str)
{
}


/* Bit */

cl_cmd_bit_arg::cl_cmd_bit_arg(class cl_cmd_arg *asfr, class cl_cmd_arg *abit_low, class cl_cmd_arg *abit_high):
  cl_cmd_arg((long)0)
{
  sfr= asfr;
  bit_low= abit_low;
  bit_high= abit_high;
}

cl_cmd_bit_arg::~cl_cmd_bit_arg(void)
{
  if (sfr)
    delete sfr;
  if (bit_low)
    delete bit_low;
  if (bit_high)
    delete bit_high;
}

bool
cl_cmd_bit_arg::get_address(class cl_uc *uc, t_addr *addr)
{
  if (sfr)
    return(sfr->get_address(uc, addr));
  return(0);
}

bool
cl_cmd_bit_arg::get_bit_address(class cl_uc *uc, // input
                                class cl_memory **mem, // outputs
                                t_addr *mem_addr,
                                int *bitnr_high,
                                int *bitnr_low)
{
  if (!sfr || !bitnr_high || !bitnr_low)
    return(false);

  if ((mem || mem_addr) && !sfr->get_bit_address(uc, mem, mem_addr, NULL, NULL))
    return(false);

  long l;

  if (!bit_low ||
      !bit_low->get_ivalue(&l) ||
      l < 0 ||
      l >= (long)sizeof(t_mem)*8)
    return(false);
  *bitnr_low = *bitnr_high = l;

  if (bit_high)
    {
      if (!bit_high->get_ivalue(&l) ||
          l < 0 ||
          l >= (long)sizeof(t_mem)*8)
        return(false);
      *bitnr_high = l;
    }

  if (*bitnr_low > *bitnr_high)
    {
      int t = *bitnr_low;
      *bitnr_low = *bitnr_high;
      *bitnr_high = t;
    }

  return(true);
}


/* Array */

cl_cmd_array_arg::cl_cmd_array_arg(class cl_cmd_arg *aname,
				   class cl_cmd_arg *aindex):
  cl_cmd_arg((long)0)
{
  name_arg= aname;
  index= aindex;
}

cl_cmd_array_arg::~cl_cmd_array_arg(void)
{
  if (name_arg)
    delete name_arg;
  if (index)
    delete index;
}

bool
cl_cmd_array_arg::get_bit_address(class cl_uc *uc, // input
                                class cl_memory **mem, // outputs
                                t_addr *mem_addr,
                                int *bitnr_high,
                                int *bitnr_low)
{
  // address_space[address]
  char *n= 0;
  t_addr a;
  if (name_arg == 0 ||
      index == 0 ||
      (n= name_arg->get_svalue()) == NULL ||
      !index->get_address(uc, &a))
    return false;
  class cl_memory *m= uc->memory(n);
  if (!m ||
      !m->valid_address(a))
    return false;
  if (mem)
    *mem = m;
  if (mem_addr)
    *mem_addr = a;
  if (bitnr_high)
    *bitnr_high = -1;
  if (bitnr_low)
    *bitnr_low = -1;
  return true;
}

bool
cl_cmd_array_arg::as_hw(class cl_uc *uc)
{
  char *n= 0;
  t_addr a;

  if (name_arg == 0 ||
      index == 0 ||
      (n= name_arg->get_svalue()) == NULL ||
      !index->get_address(uc, &a))
    return(false);

  value.hw= uc->get_hw(n, a, NULL);
  return(value.hw != NULL);
}

bool
cl_cmd_array_arg::as_cell(class cl_uc *uc)
{
  class cl_memory *mem;
  t_addr addr;

  value.cell = NULL;

  if (get_bit_address(uc, &mem, &addr, NULL, NULL) &&
      mem->is_address_space())
    value.cell= ((cl_address_space *)mem)->get_cell(addr);

  return value.cell != NULL;
}


/* End of sim.src/arg.cc */
