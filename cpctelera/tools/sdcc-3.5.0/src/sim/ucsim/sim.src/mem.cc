/*
 * Simulator of microcontrollers (mem.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "i_string.h"

// prj
#include "utils.h"
#include "globals.h"

// sim
#include "simcl.h"

// cmd
#include "newcmdcl.h"
#include "cmdutil.h"

// local
#include "memcl.h"
#include "hwcl.h"


static class cl_mem_error_registry mem_error_registry;

/*
 *                                                3rd version of memory system
 */

cl_memory::cl_memory(const char *id, t_addr asize, int awidth):
  cl_base()
{
  size= asize;
  set_name(id);
  addr_format= data_format= 0;
  width= awidth;
  start_address= 0;
  uc= 0;
}

cl_memory::~cl_memory(void)
{
  if (addr_format)
    free(addr_format);
  if (data_format)
    free(data_format);
}

int
cl_memory::init(void)
{
  addr_format= (char *)malloc(10);
  sprintf(addr_format, "0x%%0%dx",
          size-1<=0xf?1:
          (size-1<=0xff?2:
           (size-1<=0xfff?3:
            (size-1<=0xffff?4:
             (size-1<=0xfffff?5:
              (size-1<=0xffffff?6:12))))));
  data_format= (char *)malloc(10);
  sprintf(data_format, "%%0%dx", width/4+((width%4)?1:0));
  data_mask= 1;
  int w= width;
  for (--w; w; w--)
    {
      data_mask<<= 1;
      data_mask|= 1;
    }
  dump_finished= start_address;
  return(0);
}


bool
cl_memory::valid_address(t_addr addr)
{
  return(addr >= start_address &&
         addr < start_address+size);
}

t_addr
cl_memory::inc_address(t_addr addr, int val)
{
  if (!start_address)
    return(((signed)addr+val)%size);
  addr-= start_address;
  addr+= val;
  addr%= size;
  addr+= start_address;
  return(addr);
}

t_addr
cl_memory::inc_address(t_addr addr)
{
  if (!start_address)
    return(((signed)addr+1)%size);
  addr-= start_address;
  addr++;
  addr%= size;
  addr+= start_address;
  return(addr);
}

t_addr
cl_memory::validate_address(t_addr addr)
{
  while (addr < start_address)
    addr+= size;
  if (addr > start_address+size)
    {
      addr-= start_address;
      addr%= size;
      addr+= start_address;
    }
  return(addr);
}


void
cl_memory::err_inv_addr(t_addr addr)
{
  if (!uc)
    return;
  class cl_error *e= new cl_error_mem_invalid_address(this, addr);
  uc->error(e);
}

void
cl_memory::err_non_decoded(t_addr addr)
{
  if (!uc)
    return;
  class cl_error *e= new cl_error_mem_non_decoded(this, addr);
  uc->error(e);
}


t_addr
cl_memory::dump(t_addr start, t_addr stop, int bpl, class cl_console_base *con)
{
  int i;
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();

  if (start < lva)
    start= lva;
  if (stop > hva)
    stop= hva;
  while ((start <= stop) &&
         (start <= hva))
    {
      con->dd_printf(addr_format, start); con->dd_printf(" ");
      for (i= 0;
           (i < bpl) &&
             (start+i <= hva) &&
             (start+i <= stop);
           i++)
        {
          con->dd_printf(data_format, /*read*/get(start+i)); con->dd_printf(" ");
        }
      while (i < bpl)
        {
          int j;
          j= width/4 + ((width%4)?1:0) + 1;
          while (j)
            {
              con->dd_printf(" ");
              j--;
            }
          i++;
        }
      for (i= 0; (i < bpl) &&
             (start+i <= hva) &&
             (start+i <= stop);
           i++)
        {
          long c= read(start+i);
          con->dd_printf("%c", isprint(255&c)?(255&c):'.');
          if (width > 8)
            con->dd_printf("%c", isprint(255&(c>>8))?(255&(c>>8)):'.');
          if (width > 16)
            con->dd_printf("%c", isprint(255&(c>>16))?(255&(c>>16)):'.');
          if (width > 24)
            con->dd_printf("%c", isprint(255&(c>>24))?(255&(c>>24)):'.');
        }
      con->dd_printf("\n");
      dump_finished= start+i;
      start+= bpl;
    }
  return(dump_finished);
}

t_addr
cl_memory::dump(class cl_console_base *con)
{
  return(dump(dump_finished, dump_finished+10*8-1, 8, con));
}

bool
cl_memory::search_next(bool case_sensitive,
                       t_mem *array, int len, t_addr *addr)
{
  t_addr a;
  int i;
  bool found;

  if (addr == NULL)
    a= 0;
  else
    a= *addr;

  if (a+len > size)
    return(DD_FALSE);

  found= DD_FALSE;
  while (!found &&
         a+len <= size)
    {
      bool match= DD_TRUE;
      for (i= 0; i < len && match; i++)
        {
          t_mem d1, d2;
          d1= get(a+i);
          d2= array[i];
          if (!case_sensitive)
            {
              if (/*d1 < 128*/isalpha(d1))
                d1= toupper(d1);
              if (/*d2 < 128*/isalpha(d2))
                d2= toupper(d2);
            }
          match= d1 == d2;
        }
      found= match;
      if (!found)
        a++;
    }

  if (addr)
    *addr= a;
  return(found);
}


/*
 *                                                             Memory operators
 */

cl_memory_operator::cl_memory_operator(class cl_memory_cell *acell,
                                       t_addr addr):
  cl_base()
{
  cell= acell;
  data= 0;
  mask= ~0;
  next_operator= 0;
  address= addr;
}

cl_memory_operator::cl_memory_operator(class cl_memory_cell *acell,
                                       t_addr addr,
                                       t_mem *data_place, t_mem the_mask):
  cl_base()
{
  cell= acell;
  data= data_place;
  mask= the_mask;
  next_operator= 0;
  address= addr;
}

void
cl_memory_operator::set_data(t_mem *data_place, t_mem the_mask)
{
  data= data_place;
  mask= the_mask;
}


t_mem
cl_memory_operator::read(void)
{
  if (next_operator)
    return(next_operator->read());
  else
    return(*data);
}

t_mem
cl_memory_operator::write(t_mem val)
{
  if (next_operator)
    return(next_operator->write(val));
  else
    return(*data= (val & mask));
}


/* Memory operator for hw callbacks */

cl_hw_operator::cl_hw_operator(class cl_memory_cell *acell, t_addr addr,
                               t_mem *data_place, t_mem the_mask,
                               class cl_hw *ahw):
  cl_memory_operator(acell, addr, data_place, the_mask)
{
  hw= ahw;
}


t_mem
cl_hw_operator::read(void)
{
  t_mem d= 0;

  if (hw)
    d= hw->read(cell);

  if (next_operator)
    next_operator->read();

  return(d);
}

t_mem
cl_hw_operator::read(enum hw_cath skip)
{
  t_mem d= *data;

  if (hw &&
      hw->cathegory != skip)
    d= hw->read(cell);

  if (next_operator)
    next_operator->read();

  return(d);
}

t_mem
cl_hw_operator::write(t_mem val)
{
  if (hw)
    hw->write(cell, &val);
  if (next_operator)
    val= next_operator->write(val);
  return(*data= (val & mask));
}


/* Write event break on cell */

cl_write_operator::cl_write_operator(class cl_memory_cell *acell, t_addr addr,
                                     t_mem *data_place, t_mem the_mask,
                                     class cl_uc *auc, class cl_brk *the_bp):
  cl_event_break_operator(acell, addr, data_place, the_mask, auc, the_bp)
{
  uc= auc;
  bp= the_bp;
}

t_mem
cl_write_operator::write(t_mem val)
{
  //printf("write event at 0x%x bp=%p\n",address,bp);
  uc->events->add(bp);
  if (next_operator)
    return(next_operator->write(val));
  else
    return(*data= (val & mask));
}


/* Read event break on cell */

cl_read_operator::cl_read_operator(class cl_memory_cell *acell, t_addr addr,
                                   t_mem *data_place, t_mem the_mask,
                                   class cl_uc *auc, class cl_brk *the_bp):
  cl_event_break_operator(acell, addr, data_place, the_mask, auc, the_bp)
{
  uc= auc;
  bp= the_bp;
}

t_mem
cl_read_operator::read(void)
{
  //printf("read event at 0x%x bp=%p\n",address,bp);
  uc->events->add(bp);
  if (next_operator)
    return(next_operator->read());
  else
    return(*data);
}


/*
 *                                                                  Memory cell
 */

cl_memory_cell::cl_memory_cell(void):
  cl_base()
{
  data= (t_mem *)malloc(sizeof(t_mem));
  flags= CELL_NON_DECODED;
  width= 8;
  *data= 0;
  operators= NULL;

#ifdef STATISTIC
  nuof_writes= nuof_reads= 0;
#endif

  mask= 1;
  int w= width;
  for (--w; w; w--)
    {
      mask<<= 1;
      mask|= 1;
    }
}

cl_memory_cell::~cl_memory_cell(void)
{
  if ((flags & CELL_NON_DECODED) &&
      data)
    free(data);
}

int
cl_memory_cell::init(void)
{
  cl_base::init();
  set(0/*rand()*/);
  return(0);
}


TYPE_UBYTE
cl_memory_cell::get_flags(void)
{
  return(flags);
}

bool
cl_memory_cell::get_flag(enum cell_flag flag)
{
  return(flags & flag);
}

void
cl_memory_cell::set_flags(TYPE_UBYTE what)
{
  flags= what;
}

void
cl_memory_cell::set_flag(enum cell_flag flag, bool val)
{
  if (val)
    flags|= flag;
  else
    flags&= ~(flag);
}


void
cl_memory_cell::un_decode(void)
{
  if ((flags & CELL_NON_DECODED) == 0)
    {
      data= (t_mem *)malloc(sizeof(t_mem));
      flags|= CELL_NON_DECODED;
    }
}

void
cl_memory_cell::decode(class cl_memory_chip *chip, t_addr addr)
{
  if (flags & CELL_NON_DECODED)
    free(data);
  data= chip->get_slot(addr);
  if (!data)
    {
      data= (t_mem *)malloc(sizeof(t_mem));
      flags|= CELL_NON_DECODED;
    }
  else
    flags&= ~(CELL_NON_DECODED);
}


t_mem
cl_memory_cell::read(void)
{
#ifdef STATISTIC
  nuof_reads++;
#endif
  if (operators)
    return(operators->read());
  return(*data);
}

t_mem
cl_memory_cell::read(enum hw_cath skip)
{
#ifdef STATISTIC
  nuof_reads++;
#endif
  if (operators)
    return(operators->read(skip));
  return(*data);
}

t_mem
cl_memory_cell::get(void)
{
  return(*data);
}

t_mem
cl_memory_cell::write(t_mem val)
{
#ifdef STATISTIC
  nuof_writes++;
#endif
  if (operators)
    return(operators->write(val));
  *data= val & mask;
  return(*data);
}

t_mem
cl_memory_cell::set(t_mem val)
{
  *data= val & mask;
  return(*data);
}



t_mem
cl_memory_cell::add(long what)
{
  *data= (*data + what) & mask;
  return(*data);
}

t_mem
cl_memory_cell::wadd(long what)
{
  t_mem d= (*data + what) & mask;
  return(write(d));
}

void
cl_memory_cell::set_bit1(t_mem bits)
{
  bits&= mask;
  (*data)|= bits;
}

void
cl_memory_cell::set_bit0(t_mem bits)
{
  bits&= mask;
  (*data)&= ~bits;
}


void
cl_memory_cell::append_operator(class cl_memory_operator *op)
{
  if (!operators)
    operators= op;
  else
    {
      class cl_memory_operator *o= operators, *n;
      n= o->get_next();
      while (n)
        {
          o= n;
          n= o->get_next();
        }
      o->set_next(op);
    }
}

void
cl_memory_cell::prepend_operator(class cl_memory_operator *op)
{
  if (op)
    {
      op->set_next(operators);
      operators= op;
    }
}

void
cl_memory_cell::del_operator(class cl_brk *brk)
{
  if (!operators)
    return;
  class cl_memory_operator *op= operators;
  if (operators->match(brk))
    {
      operators= op->get_next();
      delete op;
    }
  else
    {
      while (op->get_next() &&
             !op->get_next()->match(brk))
        op= op->get_next();
      if (op->get_next())
        {
          class cl_memory_operator *m= op->get_next();
          op->set_next(m->get_next());;
          delete m;
        }
    }
}


class cl_memory_cell *
cl_memory_cell::add_hw(class cl_hw *hw, int *ith, t_addr addr)
{
  class cl_hw_operator *o= new cl_hw_operator(this, addr, data, mask, hw);
  append_operator(o);
  return(this);
}

/*class cl_hw *
cl_memory_cell::get_hw(int ith)
{
  return(0);
}*/

class cl_event_handler *
cl_memory_cell::get_event_handler(void)
{
  return(0);
}


/*
 * Dummy cell for non-existent addresses
 */

t_mem
cl_dummy_cell::write(t_mem val)
{
#ifdef STATISTIC
  nuof_writes++;
#endif
  *data= rand() & mask;
  return(*data);
}

t_mem
cl_dummy_cell::set(t_mem val)
{
  *data= rand() & mask;
  return(*data);
}


/*
 *                                                                Address space
 */

cl_address_space::cl_address_space(const char *id,
                                   t_addr astart, t_addr asize, int awidth):
  cl_memory(id, asize, awidth)
{
  start_address= astart;
  decoders= new cl_decoder_list(2, 2, DD_FALSE);
  cells= (class cl_memory_cell **)calloc(size, sizeof(class cl_memory_cell*));

  dummy= new cl_dummy_cell();
}

cl_address_space::~cl_address_space(void)
{
  delete decoders;
  int i;
  for (i= 0; i < size; i++)
    if (cells[i])
      delete cells[i];
  delete dummy;
}


t_mem
cl_address_space::read(t_addr addr)
{
  return get_cell(addr)->read();
}

t_mem
cl_address_space::read(t_addr addr, enum hw_cath skip)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
      return dummy->read();
    }
  return cell->read(skip);
}

t_mem
cl_address_space::get(t_addr addr)
{
  return get_cell(addr)->get();
}

t_mem
cl_address_space::write(t_addr addr, t_mem val)
{
  return get_cell(addr)->write(val);
}

void
cl_address_space::set(t_addr addr, t_mem val)
{
  get_cell(addr)->set(val);
}

t_mem
cl_address_space::wadd(t_addr addr, long what)
{
  return get_cell(addr)->wadd(what);
}

/* Set or clear bits, without callbacks */

void
cl_address_space::set_bit1(t_addr addr, t_mem bits)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
    return;
    }
  cell->set_bit1(bits);
}

void
cl_address_space::set_bit0(t_addr addr, t_mem bits)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
      return;
    }
  cell->set_bit0(bits);
}

class cl_address_decoder *
cl_address_space::get_decoder(t_addr addr)
{
  int i;
  for (i= 0; i < decoders->count; i++)
    {
      class cl_address_decoder *d=
        dynamic_cast<class cl_address_decoder *>(decoders->object_at(i));
      if (!d)
        continue;
      if (d->covers(addr, addr))
        {
          return d;
        }
    }
    return NULL;
}

class cl_memory_cell *
cl_address_space::get_cell(t_addr addr)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      return(dummy);
    }
  if (cells[idx] == NULL)
    {
      cells[idx]= new cl_memory_cell();
      cells[idx]->init();
      class cl_address_decoder *decoder;
      decoder = get_decoder(addr);
      if (decoder && decoder->activated)
        {
          decode_cell(addr, decoder->memchip,
                      addr - decoder->as_begin + decoder->chip_begin);
        }
    }
  return(cells[idx]);
}


int
cl_address_space::get_cell_flag(t_addr addr)
{
  return get_cell(addr)->get_flags();
}

bool
cl_address_space::get_cell_flag(t_addr addr, enum cell_flag flag)
{
  return get_cell(addr)->get_flag(flag);
}

void
cl_address_space::set_cell_flag(t_addr addr, bool set_to, enum cell_flag flag)
{
  get_cell(addr)->set_flag(flag, set_to);
}


bool
cl_address_space::decode_cell(t_addr addr,
                              class cl_memory_chip *chip, t_addr chipaddr)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
      return(DD_FALSE);
    }

  if (!cell->get_flag(CELL_NON_DECODED))
    {
      // un-decode first!
      cell->un_decode();
    }
  cell->decode(chip, chipaddr);

  return(!cell->get_flag(CELL_NON_DECODED));
}

void
cl_address_space::undecode_cell(t_addr addr)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      return;
    }
  if (cells[idx] == NULL)
    {
      return;
    }
  cells[idx]->un_decode();
}

void
cl_address_space::undecode_area(class cl_address_decoder *skip,
                                t_addr begin, t_addr end,class cl_console_base *con)
{
#define D if (con) con->debug
  D("Undecoding area 0x%x-0x%x of %s\n", begin, end, get_name());
  int i;
  for (i= 0; i < decoders->count; i++)
    {
      class cl_address_decoder *d=
        dynamic_cast<class cl_address_decoder *>(decoders->object_at(i));
      if (!d ||
          d == skip)
        continue;
      D("  Checking decoder 0x%x-0x%x -> %s[0x%x]\n",
        d->as_begin, d->as_end, d->memchip->get_name(), d->chip_begin);
      if (d->fully_covered_by(begin, end))
        {
          // decoder can be removed
          D("    Can be removed\n");
          decoders->disconn(d);
          i--;
          delete d;
          if (decoders->count == 0)
            break;
        }
      else if (d->covers(begin, end))
        {
          // decoder must be split
          D("    Must be split\n");
          class cl_address_decoder *nd= d->split(begin, end);
          D("    After split:\n");
          D("      0x%x-0x%x -> %s[0x%x]\n",
            d->as_begin, d->as_end, d->memchip->get_name(), d->chip_begin);
          if (nd)
            {
              decoders->add(nd);
              D("      0x%x-0x%x -> %s[0x%x]\n",
                nd->as_begin, nd->as_end, nd->memchip->get_name(), nd->chip_begin);
              nd->activate(con);
            }
        }
      else if (d->is_in(begin, end))
        {
          // decoder sould shrink
          D("    Sould shrink\n");
          if (d->shrink_out_of(begin, end))
            {
              D("    Can be removed after shrink\n");
              decoders->disconn(d);
              i--;
              delete d;
              if (decoders->count == 0)
                break;
            }
          else
            {
              D("    Shrinked to 0x%x-0x%x -> %s[0x%x]\n",
                d->as_begin, d->as_end, d->memchip->get_name(), d->chip_begin);
            }
        }
    }
#undef D
}


class cl_memory_cell *
cl_address_space::register_hw(t_addr addr, class cl_hw *hw,
                              int *ith,
                              bool announce)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
      return(NULL);
    }
  cell->add_hw(hw, ith, addr);
  //printf("adding hw %s to cell 0x%x(%d) of %s\n", hw->id_string, addr, idx, get_name("as"));
  if (announce)
    ;//uc->sim->/*app->*/mem_cell_changed(this, addr);//FIXME
  return(cell);
}


void
cl_address_space::set_brk(t_addr addr, class cl_brk *brk)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
      return;
    }

  class cl_memory_operator *op;

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
      //e= 'W';
      op= new cl_write_operator(cell, addr, cell->get_data(), cell->get_mask(),
                                uc, brk);
      break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      //e= 'R';
      op= new cl_read_operator(cell, addr, cell->get_data(), cell->get_mask(),
                               uc, brk);
      break;
    case brkNONE:
      set_cell_flag(addr, DD_TRUE, CELL_FETCH_BRK);
      return;
      break;
    default:
      //e= '.';
      op= 0;
      break;
    }
  if (op)
    cell->append_operator(op);
}

void
cl_address_space::del_brk(t_addr addr, class cl_brk *brk)
{
  cl_memory_cell *cell = get_cell(addr);
  if (cell == dummy)
    {
      return;
    }

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      cell->del_operator(brk);
      break;
    case brkNONE:
      set_cell_flag(addr, DD_FALSE, CELL_FETCH_BRK);
      return;
      break;
    default:
      break;
    }
}


/*
 * List of address spaces
 */

cl_address_space_list::cl_address_space_list(class cl_uc *the_uc):
  cl_list(2, 2, "address spaces")
{
  uc= the_uc;
}

t_index
cl_address_space_list::add(class cl_address_space *mem)
{
  mem->set_uc(uc);
  t_index ret= cl_list::add(mem);
  if (uc)
    {
      class cl_event_address_space_added e(mem);
      uc->handle_event(e);
    }
  return(ret);
}


/*
 *                                                                  Memory chip
 */

cl_memory_chip::cl_memory_chip(const char *id, int asize, int awidth, int initial):
  cl_memory(id, asize, awidth)
{
  array= (t_mem *)malloc(size * sizeof(t_mem));
  init_value= initial;
}

cl_memory_chip::~cl_memory_chip(void)
{
  if (array)
    free(array);
}

int
cl_memory_chip::init(void)
{
  cl_memory::init();
  int i;
  for (i= 0; i < size; i++)
    set(i,
        (init_value<0)?rand():(init_value));
  return(0);
}


t_mem *
cl_memory_chip::get_slot(t_addr addr)
{
  if (!array ||
      size <= addr)
    return(0);
  return(&array[addr]);
}


t_mem
cl_memory_chip::get(t_addr addr)
{
  if (!array ||
      size <= addr)
    return(0);
  return(array[addr]);
}

void
cl_memory_chip::set(t_addr addr, t_mem val)
{
  if (!array ||
      size <= addr)
    return;
  array[addr]= val & data_mask;
}

void
cl_memory_chip::set_bit1(t_addr addr, t_mem bits)
{
  if (!array ||
      size <= addr)
    return;
  array[addr]|= (bits & data_mask);
}

void
cl_memory_chip::set_bit0(t_addr addr, t_mem bits)
{
  if (!array ||
      size <= addr)
    return;
  array[addr]&= ((~bits) & data_mask);
}


/*
 *                                                              Address decoder
 */

cl_address_decoder::cl_address_decoder(class cl_memory *as,
                                       class cl_memory *chip,
                                       t_addr asb, t_addr ase, t_addr cb)
{
  if (as->is_address_space())
    address_space= (class cl_address_space *)as;
  else
    address_space= 0;
  if (chip->is_chip())
    memchip= (class cl_memory_chip *)chip;
  else
    memchip= 0;
  as_begin= asb;
  as_end= ase;
  chip_begin= cb;
  activated= DD_FALSE;
}

cl_address_decoder::~cl_address_decoder(void)
{
  t_addr a;
  if (address_space)
    for (a= as_begin; a <= as_end; a++)
      address_space->undecode_cell(a);
}

int
cl_address_decoder::init(void)
{
  return(0);
}


bool
cl_address_decoder::activate(class cl_console_base *con)
{
#define D if (con) con->debug
  D("Activation of an address decoder\n");
  if (activated)
    {
      D("Already activated\n");
      return(DD_FALSE);
    }
  if (!address_space ||
      !address_space->is_address_space())
    {
      D("No or non address space\n");
      return(DD_FALSE);
    }
  if (!memchip ||
      !memchip->is_chip())
    {
      D("No or non memory chip\n");
      return(DD_FALSE);
    }
  if (as_begin > as_end)
    {
      D("Wrong address area specification\n");
      return(DD_FALSE);
    }
  if (chip_begin >= memchip->get_size())
    {
      D("Wrong chip area specification\n");
      return(DD_FALSE);
    }
  if (as_begin < address_space->start_address ||
      as_end >= address_space->start_address + address_space->get_size())
    {
      D("Specified area is out of address space\n");
      return(DD_FALSE);
    }
  if (as_end-as_begin > memchip->get_size()-chip_begin)
    {
      D("Specified area is out of chip size\n");
      return(DD_FALSE);
    }

  address_space->undecode_area(this, as_begin, as_end, con);

  activated= DD_TRUE;

#undef D
  return(activated);
}


bool
cl_address_decoder::fully_covered_by(t_addr begin, t_addr end)
{
  if (begin <= as_begin &&
      end >= as_end)
    return(DD_TRUE);
  return(DD_FALSE);
}

bool
cl_address_decoder::is_in(t_addr begin, t_addr end)
{
  if (begin >= as_begin &&
      begin <= as_end)
    return(DD_TRUE);
  if (end >= as_begin &&
      end <= as_end)
    return(DD_TRUE);
  return(DD_FALSE);
}

bool
cl_address_decoder::covers(t_addr begin, t_addr end)
{
  if (begin > as_begin &&
      end < as_end)
    return(DD_TRUE);
  return(DD_FALSE);
}


/* Returns TRUE if shrunken decoder is unnecessary */

bool
cl_address_decoder::shrink_out_of(t_addr begin, t_addr end)
{
  t_addr a= as_begin;

  if (!address_space)
    return(DD_TRUE);
  if (begin > a)
    a= begin;
  while (a <= end &&
         a <= as_end)
    {
      address_space->undecode_cell(a);
      a++;
    }
  if (begin > as_begin)
    as_end= begin-1;
  if (as_end > end)
    {
      chip_begin+= (end-as_begin+1);
      as_begin= end+1;
    }
  if (as_end < as_begin)
    return(DD_TRUE);
  return(DD_FALSE);
}

class cl_address_decoder *
cl_address_decoder::split(t_addr begin, t_addr end)
{
  class cl_address_decoder *nd= 0;
  if (begin > as_begin)
    {
      if (as_end > end)
        nd= new cl_address_decoder(address_space, memchip,
                                   end+1, as_end, chip_begin+(end-as_begin)+1);
      shrink_out_of(begin, as_end);
    }
  else if (end < as_end)
    {
      if (as_begin < begin)
        nd= new cl_address_decoder(address_space, memchip,
                                   as_begin, begin-1, chip_begin);
      shrink_out_of(end+1, as_end);
    }
  if (nd)
    nd->init();
  return(nd);
}


/*
 * List of address decoders
 */

cl_decoder_list::cl_decoder_list(t_index alimit, t_index adelta, bool bychip):
  cl_sorted_list(alimit, adelta, "decoder list")
{
  Duplicates= DD_TRUE;
  by_chip= bychip;
}

const void *
cl_decoder_list::key_of(void *item)
{
  class cl_address_decoder *d= (class cl_address_decoder *)item;
  if (by_chip)
    return(&(d->chip_begin));
  else
    return(&(d->as_begin));
}

int
cl_decoder_list::compare(const void *key1, const void *key2)
{
  const t_addr k1= *(static_cast<const t_addr *>(key1)), k2= *(static_cast<const t_addr*>(key2));
  if (k1 == k2)
    return(0);
  else if (k1 > k2)
    return(1);
  return(-1);
}


/*
 * Errors in memory handling
 */

/* All of memory errors */

cl_error_mem::cl_error_mem(class cl_memory *amem, t_addr aaddr)
{
  mem= amem;
  addr= aaddr;
  classification= mem_error_registry.find("memory");
}

/* Invalid address in memory access */

cl_error_mem_invalid_address::
cl_error_mem_invalid_address(class cl_memory *amem, t_addr aaddr):
  cl_error_mem(amem, aaddr)
{
  classification= mem_error_registry.find("invalid_address");
}

void
cl_error_mem_invalid_address::print(class cl_commander_base *c)
{
  c->dd_printf("%s: invalid address ", get_type_name());
  c->dd_printf(mem->addr_format, addr);
  c->dd_printf(" in memory %s.\n", mem->get_name());
}

/* Non-decoded address space access */

cl_error_mem_non_decoded::
cl_error_mem_non_decoded(class cl_memory *amem, t_addr aaddr):
  cl_error_mem(amem, aaddr)
{
  classification= mem_error_registry.find("non_decoded");
}

void
cl_error_mem_non_decoded::print(class cl_commander_base *c)
{
  c->dd_printf("%s: access of non-decoded address ", get_type_name());
  c->dd_printf(mem->addr_format, addr);
  c->dd_printf(" in memory %s.\n", mem->get_name());
}

cl_mem_error_registry::cl_mem_error_registry(void)
{
  class cl_error_class *prev = mem_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "memory", prev, ERROR_OFF));
  prev = register_error(new cl_error_class(err_error, "invalid_address", prev));
  prev = register_error(new cl_error_class(err_error, "non_decoded", prev));
}


/* End of mem.cc */
