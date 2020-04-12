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
//#include <stdarg.h>
#include <string.h>
//#include "i_string.h"

// prj
//#include "utils.h"
#include "globals.h"

// sim
//#include "simcl.h"

// cmd
//#include "newcmdcl.h"
//#include "cmdutil.h"

// local
#include "memcl.h"
//#include "hwcl.h"


static class cl_mem_error_registry mem_error_registry;

/*
 *                                                3rd version of memory system
 */

cl_memory::cl_memory(const char *id, t_addr asize, int awidth):
  cl_base()
{
  if ((size= asize) > max_mem_size)
    size= max_mem_size;
  set_name(id);
  addr_format= data_format= 0;
  width= awidth;
  start_address= 0;
  uc= 0;
  hidden= false;
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
  chars c= chars("",
		 //addr_format= (char *)malloc(10);
		 /*sprintf(addr_format,*/ "0x%%0%d",
	  size-1<=0xf?1:
	  (size-1<=0xff?2:
	   (size-1<=0xfff?3:
	    (size-1<=0xffff?4:
	     (size-1<=0xfffff?5:
	      (size-1<=0xffffff?6:12))))));
  if (sizeof(t_addr) > sizeof(long))
    c+= cchars("L");//strcat(addr_format, "L");
  else if (sizeof(t_addr) > sizeof(int))
    c+= cchars("l");//strcat(addr_format, "l");
  c+= cchars("x");//strcat(addr_format, "x");
  addr_format= strdup((char*)c);
  //data_format= (char *)malloc(10);
  c= cchars("");
  /*sprintf(data_*/c.format(/*"0x"*/"%%0%d", width/4+((width%4)?1:0));
  if (sizeof(t_mem) > sizeof(long))
    c+= cchars("L");//strcat(data_format, "L");
  else if (sizeof(t_mem) > sizeof(int))
    c+= cchars("l");//strcat(data_format, "l");
  c+= cchars("x");//strcat(data_format, "x");
  data_format= strdup((char*)c);
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
cl_memory::dump(t_addr start, t_addr stop, int bpl, /*class cl_f *f*/class cl_console_base *con)
{
  int i, step;
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  class cl_f *f= con->get_fout();
  
  if (!f)
    return dump_finished;
  
  if (start < lva)
    start= lva;
  if (start > hva)
    return dump_finished;
  if (stop > hva)
    stop= hva;
  if (stop < lva)
    return dump_finished;
  if (stop >= start)
    {
      step= +1;
      stop++;
      if (start + bpl > stop)
        bpl= stop - start;
    }
  else
    {
      step= -1;
      stop--;
      if (start - bpl < stop)
        bpl= start - stop;
    }
  while ((step>0)?(start < stop):(start > stop))
    {
      // 1. field: address
      /*f->prntf*/con->dd_cprintf("dump_address", addr_format, start);
      /*f->write_str*/con->dd_printf(" ");
      // 2. field: hex list
      for (i= 0;
           (i < bpl) &&
             (start+i*step >= lva) &&
             (start+i*step <= hva) &&
             (start+i*step != stop);
           i++)
        {
          /*f->prntf*/con->dd_cprintf("dump_number", data_format, read/*get*/(start+i*step));
	  /*f->write_str*/con->dd_printf(" ");
        }
      // 3. field: char list
      while (i < bpl)
        {
	  int j;
	  j= width/4 + ((width%4)?1:0) + 1;
	  while (j)
	    {
	      /*f->write_str*/con->dd_printf(" ");
	      j--;
	    }
          i++;
        }
      for (i= 0; (i < bpl) &&
             (start+i*step >= lva) &&
             (start+i*step <= hva) &&
             (start+i*step != stop);
           i++)
        {
          long c= read(start+i*step);
          /*f->prntf*/con->dd_cprintf("dump_char", "%c", isprint(255&c)?(255&c):'.');
          if (width > 8)
            /*f->prntf*/con->dd_cprintf("dump_char", "%c", isprint(255&(c>>8))?(255&(c>>8)):'.');
          if (width > 16)
            /*f->prntf*/con->dd_cprintf("dump_char", "%c", isprint(255&(c>>16))?(255&(c>>16)):'.');
          if (width > 24)
            /*f->prntf*/con->dd_cprintf("dump_char", "%c", isprint(255&(c>>24))?(255&(c>>24)):'.');
        }
      /*f->prntf*/con->dd_printf("\n");
      dump_finished= start+i*step;
      start+= i*step;
    }
  return(dump_finished);
}

t_addr
cl_memory::dump_s(t_addr start, t_addr stop, int bpl, /*class cl_f *f*/class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  class cl_f *f= con->get_fout();

  if (!f)
    return dump_finished;
  if (start < 0)
    start= dump_finished;
  if (stop < 0)
    stop= start + 10*8 - 1;
  
  if (start < lva)
    start= lva;
  if (start > hva)
    return dump_finished;
  if (stop > hva)
    stop= hva;
  if (stop < lva)
    return dump_finished;
  
  if (bpl < 0)
    bpl= 8;
  t_addr a= start;
  t_mem d= read(a);
  char last= '\n';
  con->dd_printf("%s", (char*)(con->get_color_ansiseq("dump_char")));
  while ((a <= stop) &&
	 (d != 0) &&
	 (a <= hva))
    {
      char c= d;
      int i= d;
      chars s;
      if (a >= lva)
	{
	  switch (c)
	    { // ' " ? \ a b f n r t v
	    case '\'': s= (char*)"\\\'"; f->write((char*)s, s.len()); break;
	    case '\"': s= (char*)"\\\""; f->write((char*)s, s.len()); break;
	    case '\?': s= (char*)"\\\?"; f->write((char*)s, s.len()); break;
	    case '\\': s= (char*)"\\\\"; f->write((char*)s, s.len()); break;
	    case '\a': s= (char*)"\\a"; f->write((char*)s, s.len()); break;
	    case '\b': s= (char*)"\\b"; f->write((char*)s, s.len()); break;
	    case '\f': s= (char*)"\\f"; f->write((char*)s, s.len()); break;
	    case '\n': s= (char*)"\\n"; f->write((char*)s, s.len()); break;
	    case '\r': s= (char*)"\\r"; f->write((char*)s, s.len()); break;
	    case '\t': s= (char*)"\\t"; f->write((char*)s, s.len()); break;
	    case '\v': s= (char*)"\\v"; f->write((char*)s, s.len()); break;
	    default:
	      if (isprint(i))
		f->write(&c, 1);
	      else
		{
		  s.format("\\%03o", i);
		  f->write((char*)s, s.len());
		}
	    }
	  last= c;
	}
      d= read(++a);
    }
  if (last != '\n')
    f->write_str("\n");
  return dump_finished= a;
}

t_addr
cl_memory::dump_b(t_addr start, t_addr stop, int bpl, /*class cl_f *f*/class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  class cl_f *f= con->get_fout();

  if (!f)
    return dump_finished;
  if (start < 0)
    start= dump_finished;
  if (stop < 0)
    stop= start + 10*8 - 1;

  if (start < lva)
    start= lva;
  if (start > hva)
    return dump_finished;
  if (stop > hva)
    stop= hva;
  if (stop < lva)
    return dump_finished;

  if (bpl < 0)
    bpl= 8;
  t_addr a= start;
  t_mem d= read(a);
  while ((a <= stop) &&
	 (a <= hva))
    {
      char c= d;
      if (a >= lva)
	{
	  f->write(&c, 1);
	}
      d= read(++a);
    }
  return dump_finished= a;
}

t_addr
cl_memory::dump_i(t_addr start, t_addr stop, int bpl, /*class cl_f *f*/class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  unsigned int sum;
  t_addr start_line;
  class cl_f *f= con->get_fout();
  
  if (!f)
    return dump_finished;
  if (start < 0)
    start= dump_finished;
  if (start < lva)
    start= lva;
  if (stop < 0)
    stop= start + 10*8 - 1;
  if (stop > hva)
    stop= hva;

  if (start < lva)
    start= lva;
  if (start > hva)
    return dump_finished;
  if (stop > hva)
    stop= hva;
  if (stop < lva)
    return dump_finished;
  
  if (start > stop)
    return dump_finished= stop;
  
  if (bpl < 0)
    bpl= 16;
  if (bpl > 32)
    bpl= 32;
  t_addr a= start;
  sum= 0;
  start_line= a;
  while (a <= stop)
    {
      a++;
      if (((a % bpl) == 0) ||
	  (a > stop))
	{
	  // dump line
	  if ((a - start_line) > 0)
	    {
	      unsigned char c;	      
	      sum= 0;
	      c= a-start_line;
	      f->prntf(":%02X%04X00", c, start_line);
	      sum+= c;
	      c= int(start_line >> 8) & 0xff;
	      sum+= c;
	      c= start_line & 0xff;
	      sum+= c;
	      int i;
	      for (i= 0; i < a-start_line; i++)
		{
		  c= read(start_line + i);
		  f->prntf("%02X", c);
		  sum+= c;
		}
	      sum&= 0xff;
	      unsigned char chk= 0x100 - sum;
	      f->prntf("%02X\r\n", chk);
	    }
	  start_line= a;
	}
    }
  f->write_str(":00000001FF\r\n");
  return dump_finished= a;
}

t_addr
cl_memory::dump(/*class cl_f *f*/class cl_console_base *con)
{
  if (con->get_fout() == NULL)
    return dump_finished;
  return(dump(dump_finished, dump_finished+10*8-1, 8, /*f*/con));
}

t_addr
cl_memory::dump(enum dump_format fmt,
		t_addr start, t_addr stop, int bpl,
		/*class cl_f *f*/class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();

  //if (!f)
  //return dump_finished;
  if (start < 0)
    start= dump_finished;
  if (stop < 0)
    stop= start + 10*8 - 1;

  if (start < lva)
    start= lva;
  if (start > hva)
    return dump_finished;
  if (stop > hva)
    stop= hva;
  if (stop < lva)
    return dump_finished;
  
  if (bpl < 0)
    bpl= 8;
  switch (fmt & df_format)
    {
    case df_hex:
      return dump(start, stop, bpl, /*f*/con);
    case df_string:
      return dump_s(start, stop, bpl, /*f*/con);
    case df_ihex:
      return dump_i(start, stop, bpl, /*f*/con);
    case df_binary:
      return dump_b(start, stop, bpl, /*f*/con);
    default:
      return dump(start, stop, bpl, /*f*/con);
    }
  return dump_finished;
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
    return(false);

  found= false;
  while (!found &&
	 a+len <= size)
    {
      bool match= true;
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

void
cl_memory::print_info(chars pre, class cl_console_base *con)
{
  char *n= (char*)(get_name());
  if (!hidden)
    {
      con->dd_printf("%s0x%06x-0x%06x %8d %s (%d,%s,%s)\n", (char*)pre,
		     AU(get_start_address()),
		     AU(highest_valid_address()),
		     AU(get_size()),
		     n,
		     width, data_format, addr_format);
    }
}


/*
 *                                                             Memory operators
 */

cl_memory_operator::cl_memory_operator(class cl_memory_cell *acell/*,
								    t_addr addr*/):
  cl_base()
{
  cell= acell;
  if (cell)
    {
      //data= cell->data;
      mask= cell->mask;
    }
  else
    {
      //data= 0;
      mask= ~0;
    }
  next_operator= 0;
  //address= addr;
}
/*
cl_memory_operator::cl_memory_operator(class cl_memory_cell *acell,
				       t_addr addr,
				       t_mem *data_place, t_mem the_mask):
  cl_base()
{
  cell= acell;
  //data= data_place;
  mask= the_mask;
  next_operator= 0;
  address= addr;
}
*/
/*
void
cl_memory_operator::set_data(t_mem *data_place, t_mem the_mask)
{
  data= data_place;
  mask= the_mask;
}
*/

t_mem
cl_memory_operator::read(void)
{
  if (next_operator)
    return(next_operator->read());
  else if (cell)
    return(/* *data*/cell->get());
  return 0;
}

t_mem
cl_memory_operator::write(t_mem val)
{
  if (next_operator)
    return(next_operator->write(val));
  else if (cell)
    return(/* *data=*/cell->set(val & mask));
  return val;
}


/* Memory operator for bank switcher */

cl_bank_switcher_operator::cl_bank_switcher_operator(class cl_memory_cell *acell,
						     /*t_addr addr,*/
						     class cl_banker *the_banker):
  cl_memory_operator(acell/*, addr*/)
{
  banker= the_banker;
  set_name("bank_switcher");
}

t_mem
cl_bank_switcher_operator::write(t_mem val)
{
  if (next_operator)
    next_operator->write(val);
  if (cell) /* *data=*/ cell->set(val & mask);
  banker->activate(NULL);
  if (cell)
    return cell->get();
  return /* *data*/ 0;  
}


/* Memory operator for hw callbacks */

cl_hw_operator::cl_hw_operator(class cl_memory_cell *acell/*, t_addr addr*/,
			       //t_mem *data_place, t_mem the_mask,
			       class cl_hw *ahw):
  cl_memory_operator(acell/*, addr*//*, data_place, the_mask*/)
{
  hw= ahw;
  set_name(chars("hw:")+hw->get_name());
}


t_mem
cl_hw_operator::read(void)
{
  t_mem d1= 0, d2= 0;

  if (hw)
    d1= hw->read(cell);

  if (next_operator)
    d2= next_operator->read();

  return(hw?d1:d2);
}

t_mem
cl_hw_operator::read(enum hw_cath skip)
{
  t_mem d1= 0/* *data*/, d2= d1;
  bool use= false;

  if (hw &&
      hw->cathegory != skip)
    use= true, d1= hw->read(cell);

  if (next_operator)
    d2= next_operator->read();
  else if (cell)
    d2= cell->get();
  else
    return use= true;

  return(use?d1:d2);
}

t_mem
cl_hw_operator::write(t_mem val)
{
  if (hw)
    hw->write(cell, &val);
  if (next_operator)
    val= next_operator->write(val);
  //if (cell) return(/* *data=*//*cell->set(val & mask)*/val);
  return val;
}


/* Write event break on cell */

cl_write_operator::cl_write_operator(class cl_memory_cell *acell/*, t_addr addr*/,
				     //t_mem *data_place, t_mem the_mask,
				     class cl_uc *auc, class cl_brk *the_bp):
  cl_event_break_operator(acell/*, addr*//*, data_place, the_mask*/, auc, the_bp)
{
  uc= auc;
  bp= the_bp;
  set_name("write_event");
}

t_mem
cl_write_operator::write(t_mem val)
{
  //printf("write event at 0x%x bp=%p\n",address,bp);
  if (bp->do_hit())
    uc->events->add(bp);
  if (next_operator)
    return(next_operator->write(val));
  else if (cell)
    return(/* *data=*/cell->set(val & mask));
  return val;
}


/* Read event break on cell */

cl_read_operator::cl_read_operator(class cl_memory_cell *acell/*, t_addr addr*/,
				   //t_mem *data_place, t_mem the_mask,
				   class cl_uc *auc, class cl_brk *the_bp):
  cl_event_break_operator(acell/*, addr*//*, data_place, the_mask*/, auc, the_bp)
{
  uc= auc;
  bp= the_bp;
  set_name("read_event");
}

t_mem
cl_read_operator::read(void)
{
  //printf("read event at 0x%x bp=%p\n",address,bp);
  if (bp->do_hit())
    uc->events->add(bp);
  if (next_operator)
    return(next_operator->read());
  else if (cell)
    return(/* *data*/cell->get());
  return 0;
}


/*
 *                                                                  Cell data
 */

t_mem
cl_cell_data::d()
{
  return data?(*data):0;
}

void
cl_cell_data::d(t_mem v)
{
  data?(*data=v):0;
}

void
cl_cell_data::dl(t_mem v)
{
  data?(*data=v):0;
}

// bit cell for bit spaces

t_mem
cl_bit_cell::d()
{
  if (!data)
    return 0;
  return (*data&mask)?1:0;
}

void
cl_bit_cell::d(t_mem v)
{
  if (!data)
    return;
  if (v)
    *data|= mask;
  else
    *data&= ~mask;
}


// 8 bit cell;

t_mem
cl_cell8::d()
{
  return data?((/*u8_t*/uchar)*data):0;
}

void
cl_cell8::d(t_mem v)
{
  data?(*data=(/*u8_t*/uchar)v):0;
}

// 8 bit cell for bit spaces

t_mem
cl_bit_cell8::d()
{
  if (!data)
    return 0;
  /*u8_t*/uchar x= (/*u8_t*/uchar) *data;
  x&= mask;
  return x?1:0;
}

void
cl_bit_cell8::d(t_mem v)
{
  if (!data)
    return;
  if (v)
    *data |= (/*u8_t*/uchar)mask;
  else
    *data &= ~(/*u8_t*/uchar)mask;
}

// 16 bit cell;

t_mem
cl_cell16::d()
{
  return data?((u16_t)*data):0;
}

void
cl_cell16::d(t_mem v)
{
  data?(*data=(u16_t)v):0;
}

// 16 bit cell for bit spaces

t_mem
cl_bit_cell16::d()
{
  if (!data)
    return 0;
  return (((u16_t)*data)&((u16_t)mask))?1:0;
}

void
cl_bit_cell16::d(t_mem v)
{
  if (!data)
    return;
  if (v)
    *data |= (u16_t)mask;
  else
    *data &= ~(u16_t)mask;
}


/*
 *                                                                  Memory cell
 */

cl_memory_cell::cl_memory_cell(uchar awidth)//: cl_base()
{
  data= 0;
  flags= CELL_NON_DECODED;
  width= awidth;
  //*data= 0;
  def_data= 0;
  operators= NULL;
  //bank= 0;
  //banked_data_ptrs= 0;
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
    ;//free(data);
}

int
cl_memory_cell::init(void)
{
  //cl_base::init();
  data= &def_data;
  //flags= CELL_NON_DECODED;
  /*mask= 1;
  int w= width;
  for (--w; w; w--)
    {
      mask<<= 1;
      mask|= 1;
      }*/
  //set(0/*rand()*/);
  return(0);
}


uchar
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
cl_memory_cell::set_flags(uchar what)
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
      data= &def_data;//(t_mem *)malloc(sizeof(t_mem));
      flags|= CELL_NON_DECODED;
    }
}

void
cl_memory_cell::decode(class cl_memory_chip *chip, t_addr addr)
{
  if (flags & CELL_NON_DECODED)
    ;//free(data);
  data= chip->get_slot(addr);
  if (!data)
    {
      data= &def_data;//(t_mem *)malloc(sizeof(t_mem));
      flags|= CELL_NON_DECODED;
    }
  else
    flags&= ~(CELL_NON_DECODED);
}

void
cl_memory_cell::decode(t_mem *data_ptr)
{
  if (data_ptr == NULL)
    {
      data= &def_data;
      flags|= CELL_NON_DECODED;
    }
  else
    {
      data= data_ptr;
      flags&= ~CELL_NON_DECODED;
    }
}

void
cl_memory_cell::decode(t_mem *data_ptr, t_mem bit_mask)
{
  if (data_ptr == NULL)
    {
      data= &def_data;
      flags|= CELL_NON_DECODED;
    }
  else
    {
      data= data_ptr;
      flags&= ~CELL_NON_DECODED;
    }
  mask= bit_mask;
}
  
t_mem
cl_memory_cell::read(void)
{
#ifdef STATISTIC
  nuof_reads++;
#endif
  if (operators)
    return(operators->read());
  return /* *data*/d();
}

t_mem
cl_memory_cell::read(enum hw_cath skip)
{
#ifdef STATISTIC
  nuof_reads++;
#endif
  if (operators)
    return(operators->read(skip));
  return /* *data*/d();
}

t_mem
cl_memory_cell::get(void)
{
  return /* *data*/d();
}

t_mem
cl_memory_cell::write(t_mem val)
{
#ifdef STATISTIC
  nuof_writes++;
#endif
  if (operators)
    val= operators->write(val);
  if (flags & CELL_READ_ONLY)
    return d();
  if (width == 1)
    d(val);
  else
    d(val & mask);
  return d();
}

t_mem
cl_memory_cell::set(t_mem val)
{
  if (flags & CELL_READ_ONLY)
    return d();
  if (width == 1)
    d(val);
  else
    /* *data=*/d(val & mask);
  return /* *data*/d();
}

t_mem
cl_memory_cell::download(t_mem val)
{
  if (width == 1)
    dl(val);
  else
    /* *data=*/dl(val & mask);
  return /* *data*/d();
}

t_mem
cl_memory_cell::add(long what)
{
  /* *data=*/ d( (*data + what) & mask);
  return(/* *data*/d());
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
  /*(*data)|=*//*d*/set(d()| bits);
}

void
cl_memory_cell::write_bit1(t_mem bits)
{
  bits&= mask;
  /*(*data)|=*//*d*/write(d()| bits);
}

void
cl_memory_cell::set_bit0(t_mem bits)
{
  bits&= mask;
  /*(*data)&=*//*d*/set(d()& ~bits);
}

void
cl_memory_cell::write_bit0(t_mem bits)
{
  bits&= mask;
  /*(*data)&=*//*d*/write(d()& ~bits);
}

void
cl_memory_cell::toggle_bits(t_mem bits)
{
  bits&= mask;
  /*d*/set(d() ^ bits);
}

void
cl_memory_cell::wtoggle_bits(t_mem bits)
{
  bits&= mask;
  /*d*/write(d() ^ bits);
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

void 	 
cl_memory_cell::del_operator(class cl_hw *hw)
{
  if (!operators)
    return;
  class cl_memory_operator *op= operators;
  if (operators->match(hw))
    {
      operators= op->get_next();
      delete op;
    }
  else
    {
      while (op->get_next() &&
	     !op->get_next()->match(hw))
	op= op->get_next();
      if (op->get_next())
	{
	  class cl_memory_operator *m= op->get_next();
	  op->set_next(m->get_next());
	  delete m;
	}
    }
}

class cl_banker *
cl_memory_cell::get_banker(void)
{
  class cl_memory_operator *op= operators;
  class cl_banker *b= NULL;

  while (op)
    {
      b= op->get_banker();
      if (b)
	return b;
      op= op->get_next();
    }
  return NULL;
}

class cl_memory_cell *
cl_memory_cell::add_hw(class cl_hw *hw/*, t_addr addr*/)
{
  class cl_hw_operator *o= new cl_hw_operator(this/*, addr*//*, data, mask*/, hw);
  append_operator(o);
  return(this);
}

void 	 
cl_memory_cell::remove_hw(class cl_hw *hw) 	 
{ 	 
  del_operator(hw); 	 
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

void
cl_memory_cell::print_info(chars pre, class cl_console_base *con)
{
  con->dd_printf("%sFlags:", (char*)pre);
  if (flags & CELL_VAR)
    con->dd_printf(" VAR");
  if (flags & CELL_INST)
    con->dd_printf(" INST");
  if (flags & CELL_FETCH_BRK)
    con->dd_printf(" FBRK");
  if (flags & CELL_READ_ONLY)
    con->dd_printf(" RO");
  if (flags & CELL_NON_DECODED)
    con->dd_printf(" NDC");
  con->dd_printf("\n");
  print_operators(pre, con);
}

void
cl_memory_cell::print_operators(chars pre, class cl_console_base *con)
{
  class cl_memory_operator *o= operators;
  if (!operators)
    return;
  int i= 0;
  while (o)
    {
      printf("%s %02d. %s\n", (char*)pre, i, o->get_name("?"));
      i++;
      o= o->get_next();
    }
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
  class cl_memory_cell c(awidth);
  class cl_bit_cell8 bc8(awidth);
  class cl_cell8 c8(awidth);
  class cl_cell16 c16(awidth);
  class cl_memory_cell *cell= &c;
  start_address= astart;
  decoders= new cl_decoder_list(2, 2, false);
  cella= (class cl_memory_cell *)malloc(size * sizeof(class cl_memory_cell));
  if (awidth == 1)
    cell= &bc8;
  else if (awidth <= 8)
    cell= &c8;
  else if (awidth <= 16)
    cell= &c16;
  //cell->init();
  int i;
  for (i= 0; i < size; i++)
    {
      void *p1= &(cella[i]);
      void *p2= cell;
      memcpy(p1, p2, sizeof(c));
      cella[i].init();
    }
  dummy= new cl_dummy_cell(awidth);
  dummy->init();
}

cl_address_space::~cl_address_space(void)
{
  delete decoders;
  int i;
  for (i= 0; i < size; i++)
    {
      cella[i].~cl_memory_cell();
    }
  free(cella);
  delete dummy;
}


t_mem
cl_address_space::read(t_addr addr)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      return(dummy->read());
    }
  return(cella[idx].read());
}

t_mem
cl_address_space::read(t_addr addr, enum hw_cath skip)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      return(dummy->read());
    }
  return(cella[idx].read(skip));
}

t_mem
cl_address_space::get(t_addr addr)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      return(dummy->get());
    }
  return cella[idx].get();//*(cella[idx].data);
}

t_mem
cl_address_space::write(t_addr addr, t_mem val)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      return(dummy->write(val));
    }
  //if (cella[idx].get_flag(CELL_NON_DECODED)) printf("%s[%d] nondec write=%x\n",get_name(),addr,val);
  return(cella[idx].write(val));
}

void
cl_address_space::set(t_addr addr, t_mem val)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      dummy->set(val);
      return;
    }
  /* *(cella[idx].data)=*/cella[idx].set( val/*&(data_mask)*/);
}

void
cl_address_space::download(t_addr addr, t_mem val)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
      dummy->download(val);
      return;
    }
  /* *(cella[idx].data)=*/cella[idx].download( val/*&(data_mask)*/);
}

t_mem
cl_address_space::wadd(t_addr addr, long what)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      err_inv_addr(addr);
    }
  return(cella[idx].wadd(what));
}

/* Set or clear bits, without callbacks */

void
cl_address_space::set_bit1(t_addr addr, t_mem bits)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return;
  class cl_memory_cell *cell= &(cella[idx]);
  cell->set_bit1(bits);
}

void
cl_address_space::set_bit0(t_addr addr, t_mem bits)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return;
  class cl_memory_cell *cell= &(cella[idx]);
  cell->set_bit0(bits);
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
  return(&cella[idx]);
}


int
cl_address_space::get_cell_flag(t_addr addr)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      return(dummy->get_flags());
    }
  return(cella[idx].get_flags());
}

bool
cl_address_space::get_cell_flag(t_addr addr, enum cell_flag flag)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    {
      return(dummy->get_flag(flag));
    }
  return(cella[idx].get_flag(flag));
}

void
cl_address_space::set_cell_flag(t_addr addr, bool set_to, enum cell_flag flag)
{
  t_addr idx= addr-start_address;
  class cl_memory_cell *cell;
  
  if (idx >= size ||
      addr < start_address)
    {
      cell= dummy;
    }
  else
    cell= &cella[idx];
  cell->set_flag(flag, set_to);
}

void
cl_address_space::set_cell_flag(t_addr start_addr, t_addr end_addr, bool set_to, enum cell_flag flag)
{
  t_addr a;

  for (a= start_addr; a <= end_addr; a++)
    set_cell_flag(a, set_to, flag);
}

class cl_memory_cell *
cl_address_space::search_cell(enum cell_flag flag, bool value, t_addr *addr)
{
  int i;

  for (i= 0; i < size; i++)
    {
      bool f= cella[i].get_flag(flag);
      if ((f && value) ||
	  (!f && !value))
	{
	  if (addr)
	    *addr= i;
	  return &cella[i];
	}
    }
  return NULL;
}

bool
cl_address_space::is_owned(class cl_memory_cell *cell, t_addr *addr)
{
  if (cell < cella)
    return false;
  if (cell > &cella[size-1])
    return false;
  int idx= cell - cella;
  if (addr)
    *addr= start_address+idx;
  return true;
}

class cl_address_decoder *
cl_address_space::get_decoder_of(t_addr addr)
{
  class cl_address_decoder *dc;
  int i;
  for (i= 0; i < decoders->count; i++)
    {
      dc= (class cl_address_decoder *)(decoders->at(i));
      if (dc->covers(addr, addr))
	return dc;
    }
  return NULL;
}
  
bool
cl_address_space::decode_cell(t_addr addr,
			      class cl_memory_chip *chip, t_addr chipaddr)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return(false);
  class cl_memory_cell *cell= &cella[idx];

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
    return;
  class cl_memory_cell *cell= &cella[idx];

  cell->un_decode();
}

void
cl_address_space::undecode_area(class cl_address_decoder *skip,
				t_addr begin, t_addr end,
				class cl_console_base *con)
{
#define D if (con) con->debug
  //#define D printf
  D("Undecoding area 0x%lx-0x%lx of %s (skip=%s)\n", begin, end, get_name(), skip?(skip->get_name()):"-");
  int i;
  for (i= 0; i < decoders->count; i++)
    {
      class cl_address_decoder *d=
	dynamic_cast<class cl_address_decoder *>(decoders->object_at(i));
      if (!d ||
	  d == skip)
	continue;
      D("  Checking decoder 0x%lx-0x%lx -> %s[0x%lx]\n",
	d->as_begin, d->as_end, (d->memchip)?(d->memchip->get_name()):"(none)", d->chip_begin);
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
	  D("      0x%lx-0x%lx -> %s[0x%lx]\n",
	    d->as_begin, d->as_end, (d->memchip)?(d->memchip->get_name()):"(none)", d->chip_begin);
	  if (nd)
	    {
	      decoders->add(nd);
	      D("      0x%lx-0x%lx -> %s[0x%lx]\n",
		nd->as_begin, nd->as_end, (nd->memchip)?(nd->memchip->get_name()):"none", nd->chip_begin);
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
	      D("    Shrinked to 0x%lx-0x%lx -> %s[0x%lx]\n",
		d->as_begin, d->as_end, (d->memchip)?(d->memchip->get_name()):"(none)", d->chip_begin);
	    }
	}
    }
#undef D
}


class cl_memory_cell *
cl_address_space::register_hw(t_addr addr, class cl_hw *hw,
			      bool announce)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return(0);
  class cl_memory_cell *cell= &cella[idx];
  cell->add_hw(hw/*, addr*/);
  if (announce)
    ;//uc->sim->/*app->*/mem_cell_changed(this, addr);//FIXME
  return(cell);
}

void 	 
cl_address_space::unregister_hw(class cl_hw *hw)
{
  t_addr idx;

  for (idx= 0; idx < size; idx++)
    {
      class cl_memory_cell *cell= &cella[idx];
      cell->remove_hw(hw);
    }
}

void
cl_address_space::set_brk(t_addr addr, class cl_brk *brk)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return;
  class cl_memory_cell *cell= &cella[idx];
  class cl_memory_operator *op;

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
      //e= 'W';
      op= new cl_write_operator(cell/*, addr*/, //cell->get_data(), cell->get_mask(),
				uc, brk);
      break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      //e= 'R';
      op= new cl_read_operator(cell/*, addr*/, //cell->get_data(), cell->get_mask(),
			       uc, brk);
      break;
    case brkNONE:
      set_cell_flag(addr, true, CELL_FETCH_BRK);
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
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return;
  class cl_memory_cell *cell= &cella[idx];

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      cell->del_operator(brk);
      break;
    case brkNONE:
      set_cell_flag(addr, false, CELL_FETCH_BRK);
      return;
      break;
    default:
      break;
    }
}

void
cl_address_space::print_info(chars pre, class cl_console_base *con)
{
  char *n= (char*)(get_name());
  if (!hidden)
    {
      con->dd_printf("%s0x%06x-0x%06x %8d %s (%d,%s,%s)\n", (char*)pre,
		     AU(get_start_address()),
		     AU(highest_valid_address()),
		     AU(get_size()),
		     n,
		     width, data_format, addr_format);
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

cl_memory_chip::cl_memory_chip(const char *id,
			       int asize,
			       int awidth,
			       int initial):
  cl_memory(id, asize, awidth)
{
  array= (t_mem *)malloc(size * sizeof(t_mem));
  init_value= initial;
  array_is_mine= true;
}

cl_memory_chip::cl_memory_chip(const char *id,
			       int asize,
			       int awidth,
			       t_mem *aarray):
  cl_memory(id, asize, awidth)
{
  array= aarray;
  init_value= 0;
  array_is_mine= false;
}

cl_memory_chip::~cl_memory_chip(void)
{
  if (array &&
      array_is_mine)
    free(array);
}

int
cl_memory_chip::init(void)
{
  cl_memory::init();
  int i;
  if (array_is_mine)
    {
      for (i= 0; i < size; i++)
	set(i,
	    (init_value<0)?rand():(init_value)
	    );
    }
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

t_addr
cl_memory_chip::is_slot(t_mem *data_ptr)
{
  if (data_ptr < &(array[0]))
    return -1;
  if (data_ptr > &(array[size-1]))
    return -2;
  return data_ptr - &(array[0]);
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

void
cl_memory_chip::print_info(chars pre, class cl_console_base *con)
{
  char *n= (char*)(get_name());
  if (!hidden)
    {
      //con->dd_printf(pre0);
      con->dd_printf("%s0x%06x-0x%06x %8d %s (%d,%s,%s)\n", (char*)pre,
		     AU(get_start_address()),
		     AU(highest_valid_address()),
		     AU(get_size()),
		     n,
		     width, data_format, addr_format);
    }
}


/*
 *                                                              Address decoder
 */

cl_address_decoder::cl_address_decoder(class cl_memory *as,
				       class cl_memory *chip,
				       t_addr asb, t_addr ase, t_addr cb)
{
  if (as && (as->is_address_space()))
    address_space= (class cl_address_space *)as;
  else
    address_space= 0;
  if (chip && (chip->is_chip()))
    memchip= (class cl_memory_chip *)chip;
  else
    memchip= 0;
  as_begin= asb;
  as_end= ase;
  chip_begin= cb;
  activated= false;
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
  //#define D printf
  D("Activation of an address decoder %s (%s[%06lx-%06lx]\n", get_name(""), address_space->get_name(), as_begin, as_end);
  if (activated)
    {
      D("Already activated\n");
      return(false);
    }
  if (!address_space ||
      !address_space->is_address_space())
    {
      D("No or non address space\n");
      return(false);
    }
  if (!memchip ||
      !memchip->is_chip())
    {
      D("No or non memory chip\n");
      return(false);
    }
  if (as_begin > as_end)
    {
      D("Wrong address area specification\n");
      return(false);
    }
  if (chip_begin >= memchip->get_size())
    {
      D("Wrong chip area specification\n");
      return(false);
    }
  if (as_begin < address_space->start_address ||
      as_end >= address_space->start_address + address_space->get_size())
    {
      D("Specified area is out of address space\n");
      return(false);
    }
  if (as_end-as_begin > memchip->get_size()-chip_begin)
    {
      D("Specified area is out of chip size\n");
      return(false);
    }

  address_space->undecode_area(this, as_begin, as_end, con);

  D("Decoder maps %s[%06lx-%06lx] -> %s[%06lx]...\n",address_space->get_name(),as_begin,as_end,memchip->get_name(),chip_begin);
  t_addr asa, ca;
  for (asa= as_begin, ca= chip_begin;
       asa <= as_end;
       asa++, ca++)
    {
      if (!address_space->decode_cell(asa, memchip, ca))
	{
	  D("Decoding 0x%06lx->0x%06lx failed\n", asa, ca);
	}
    }
  activated= true;

#undef D
  return(activated);
}

/* Check if this DEC is fully within the specified area

   as_begin....................as_end
 ^                                    ^
 begin                              end

*/

bool
cl_address_decoder::fully_covered_by(t_addr begin, t_addr end)
{
  if (begin <= as_begin &&
      end >= as_end)
    return(true);
  return(false);
}

/* Check if some part of this DEC is in the specified area:

   as_begin......................as_end
                         ^               ^
                         begin         end

   as_begin......................as_end
^               ^
begin           end

*/

bool
cl_address_decoder::is_in(t_addr begin, t_addr end)
{
  if (begin >= as_begin &&
      begin <= as_end)
    return(true);
  if (end >= as_begin &&
      end <= as_end)
    return(true);
  return(false);
}

/* Check if this DEC covers the specified area:

   as_begin....................as_end
             ^             ^
             begin       end

*/

bool
cl_address_decoder::covers(t_addr begin, t_addr end)
{
  if (begin >= as_begin &&
      end <= as_end)
    return(true);
  return(false);
}


/* Returns TRUE if shrunken decoder is unnecessary */

bool
cl_address_decoder::shrink_out_of(t_addr begin, t_addr end)
{
  t_addr a= as_begin;

  if (!address_space)
    return(true);
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
    return(true);
  return(false);
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
      shrink_out_of(as_begin, end);
    }
  if (nd)
    nd->init();
  return(nd);
}

void
cl_address_decoder::print_info(chars pre, class cl_console_base *con)
{
  if (address_space &&
      address_space->hidden)
    return;
  if (memchip &&
      memchip->hidden)
    return;
  con->dd_printf(pre);
  if (address_space)
    {
      con->dd_printf("%s ", address_space->get_name("unknown"));
      con->dd_printf(address_space->addr_format, as_begin);
      con->dd_printf(" ");
      con->dd_printf(address_space->addr_format, as_end);
    }
  else
    con->dd_printf("x");
  con->dd_printf(" -> ");
  if (memchip)
    {
      con->dd_printf("%s ", memchip->get_name("unknown"));
      con->dd_printf(memchip->addr_format, chip_begin);
    }
  else
    con->dd_printf("x");
  con->dd_printf(" %s\n", (activated)?"activated":"inactive");
}


/*
 * Bank switcher
 */

cl_banker::cl_banker(class cl_address_space *the_banker_as,
		     t_addr the_banker_addr,
		     t_mem the_banker_mask,
		     //int the_banker_shift,
		     class cl_address_space *the_as,
		     t_addr the_asb,
		     t_addr the_ase):
  cl_address_decoder(the_as, NULL, the_asb, the_ase, (t_addr)-1)
{
  banker_as= the_banker_as;
  banker_addr= the_banker_addr;
  banker_mask= the_banker_mask;
  //banker_shift= the_banker_shift;
  banker2_as= NULL;
  banker2_addr= 0;
  banker2_mask= 0;
  banker2_shift= 0;
  nuof_banks= 0;
  banks= 0;
  //bank_ptrs= 0;
  bank= -1;
}

cl_banker::cl_banker(class cl_address_space *the_banker_as,
		     t_addr the_banker_addr,
		     t_mem the_banker_mask,
		     //int the_banker_shift,
		     class cl_address_space *the_banker2_as,
		     t_addr the_banker2_addr,
		     t_mem the_banker2_mask,
		     int the_banker2_shift,
		     class cl_address_space *the_as,
		     t_addr the_asb,
		     t_addr the_ase):
  cl_address_decoder(the_as, NULL, the_asb, the_ase, (t_addr)-1)
{
  banker_as= the_banker_as;
  banker_addr= the_banker_addr;
  banker_mask= the_banker_mask;
  //banker_shift= the_banker_shift;
  banker2_as= the_banker2_as;
  banker2_addr= the_banker2_addr;
  banker2_mask= the_banker2_mask;
  banker2_shift= the_banker2_shift;
  nuof_banks= 0;
  banks= 0;
  //bank_ptrs= 0;
  bank= -1;
}

int
cl_banker::init()
{
  int m= banker_mask;
  int b, b2;

  shift_by= 0;
  shift2_by= 0;
  if (m == 0)
    nuof_banks= 0;
  else
    {
      while ((m&1) == 0)
	m>>= 1, shift_by++;
      b= 1;
      m>>= 1;
      while ((m&1) != 0)
	{
	  m>>= 1;
	  b++;
	}
      nuof_banks= 1 << b;
    }
  shift2_by= 0;
  if (banker2_as &&
      banker2_mask)
    {
      m= banker2_mask;
      while ((m&1) == 0)
	m>>=1, shift2_by++;
      b2= 1;
      m>>= 1;
      while ((m&1) != 0)
	m>>= 1, b2++;
      if (b2)
	nuof_banks*= (1 << b2);
    }
  if (nuof_banks > 0)
    {
      banks= (class cl_address_decoder **)malloc(nuof_banks * sizeof(class cl_address_decoder *));
      //bank_ptrs= (t_mem **)calloc(nuof_banks*(as_end-as_begin+1), sizeof(t_mem *));
      for (b= 0; b < nuof_banks; b++)
	{
	  banks[b]= NULL;
	}
    }

  class cl_memory_cell *c= banker_as->get_cell(banker_addr);
  if (c)
    {
      class cl_bank_switcher_operator *o=
	new cl_bank_switcher_operator(c/*, banker_addr*/, this);
      c->prepend_operator(o);
    }
  if (banker2_as &&
      banker2_mask)
    {
      c= banker2_as->get_cell(banker2_addr);
      if (c)
	{
	  class cl_bank_switcher_operator *o=
	    new cl_bank_switcher_operator(c/*, banker_addr*/, this);
	  c->prepend_operator(o);
	}
    }
  return 0;
}

cl_banker::~cl_banker()
{
  int i;
  if (banks)
    {
      for (i= 0; i < nuof_banks; i++)
	{
	  if (banks[i])
	    delete banks[i];
	}
      free(banks);
    }
  //if (bank_ptrs) free(bank_ptrs);
}

void
cl_banker::add_bank(int bank_nr, class cl_memory *chip, t_addr chip_start)
{
  if (!chip)
    return;
  if (!address_space)
    return;
  if (!chip->is_chip())
    return;

  if (bank_nr >= nuof_banks)
    return;
  
  class cl_address_decoder *ad= new cl_address_decoder(address_space,
						       chip,
						       as_begin, as_end,
						       chip_start);
  ad->init();
  if (banks[bank_nr])
    {
      delete banks[bank_nr];
      banks[bank_nr]= 0;
    }
  banks[bank_nr]= ad;
  /*
  t_addr a, s, i;
  s= as_end - as_begin + 1;
  for (i= 0; i < s; i++)
    {
      a= chip_start + i;
      //bank_ptrs[bank_nr*s + i]= ad->memchip->get_slot(a);
    }
  */
  activate(0);
}

t_mem
cl_banker::actual_bank()
{
  //t_mem m= banker_mask;
  t_mem v= banker_as->read(banker_addr) & banker_mask;
  t_mem v2;
  
  v= (v >> shift_by);
  if (banker2_as &&
      banker2_mask)
    {
      v2= banker2_as->read(banker2_addr) & banker2_mask;
      v2>>= shift2_by;
      v2= v2 << banker2_shift;
      v= v | v2;
    }
  return v;
}

bool
cl_banker::activate(class cl_console_base *con)
{
  int b= actual_bank();
  t_addr i, s;
  t_mem *data;
  class cl_memory_cell *c;

  if (b == bank)
    return true;
  if (banks[b] == NULL)
    return true;
  s= as_end - as_begin + 1;
  for (i= 0; i < s; i++)
    {
      t_addr ca= banks[b]->chip_begin + i;
      data= banks[b]->memchip->get_slot(ca);
      c= address_space->get_cell(as_begin+i);
      c->decode(data);
    }
  bank= b;

  return true;
}

bool
cl_banker::switch_to(int bank_nr, class cl_console_base *con)
{
  int b= bank_nr;//actual_bank();
  t_addr i, s;
  t_mem *data;
  class cl_memory_cell *c;

  if (b == bank)
    return true;
  if (banks[b] == NULL)
    return true;
  s= as_end - as_begin + 1;
  for (i= 0; i < s; i++)
    {
      t_addr ca= banks[b]->chip_begin + i;
      data= banks[b]->memchip->get_slot(ca);
      c= address_space->get_cell(as_begin+i);
      c->decode(data);
    }
  bank= b;

  return true;
}

void
cl_banker::print_info(chars pre, class cl_console_base *con)
{
  int b;
  con->dd_printf(pre);
  //con->dd_printf("  banked area= ");
  if (address_space)
    {
      con->dd_printf("%s ", address_space->get_name("unknown"));
      con->dd_printf(address_space->addr_format, as_begin);
      con->dd_printf(" ");
      con->dd_printf(address_space->addr_format, as_end);
    }
  else
    con->dd_printf("x");
  con->dd_printf(" -> banked\n");

  con->dd_printf(pre);
  con->dd_printf("  bank selector: %s[", banker_as->get_name("unknown"));
  con->dd_printf(banker_as->addr_format, banker_addr);
  con->dd_printf("] mask=0x%x banks=%d act=%d\n",
		 banker_mask, nuof_banks,
		 b= actual_bank());

  con->dd_printf(pre);
  con->dd_printf("  banks:\n");

  class cl_address_decoder *dc;
  int i;
  for (i= 0; i < nuof_banks; i++)
    {
      dc= (class cl_address_decoder *)(banks[i]);
      con->dd_printf(pre);
      con->dd_printf("    %c %2d. ", (b==i)?'*':' ', i);
      if (dc)
	{
	  if (dc->memchip)
	    {
	      con->dd_printf("%s ", dc->memchip->get_name("unknown"));
	      con->dd_printf(dc->memchip->addr_format, dc->chip_begin);
	    }
	  else
	    con->dd_printf("x");
	}
      else
	con->dd_printf("-");
      con->dd_printf("\n");
    }
}


/* 
 * Bit bander
 */

cl_bander::cl_bander(class cl_address_space *the_as,
		     t_addr the_asb,
		     t_addr the_ase,
		     class cl_memory *the_chip,
		     t_addr the_cb,
		     int the_bpc,
		     int the_distance):
  cl_address_decoder(the_as, the_chip, the_asb, the_ase, the_cb)
{
  bpc= the_bpc;
  distance= the_distance;
}

bool
cl_bander::activate(class cl_console_base *con)
{
  address_space->undecode_area(this, as_begin, as_end, con);

  t_addr asa, ca;
  int b, m;
  for (asa= as_begin, ca= chip_begin, b= 0, m= 1;
       asa <= as_end;
       asa++)
    {
      if (b >= bpc)
	{
	  ca+= distance;
	  b= 0;
	  m= 1;
	}
      t_mem *slot= memchip->get_slot(ca);
      cl_memory_cell *c= address_space->get_cell(asa);
      c->decode(slot, m);
      b++;
      m<<= 1;
    }
  return activated= true;
}

void
cl_bander::print_info(chars pre, class cl_console_base *con)
{
  if (address_space &&
      address_space->hidden)
    return;
  if (memchip &&
      memchip->hidden)
    return;
  con->dd_printf(pre);
  if (address_space)
    {
      con->dd_printf("%s ", address_space->get_name("unknown"));
      con->dd_printf(address_space->addr_format, as_begin);
      con->dd_printf(" ");
      con->dd_printf(address_space->addr_format, as_end);
    }
  else
    con->dd_printf("x");
  con->dd_printf(" -> bander(%d/%d) ", bpc, distance);
  if (memchip)
    {
      con->dd_printf("%s ", memchip->get_name("unknown"));
      con->dd_printf(memchip->addr_format, chip_begin);
    }
  else
    con->dd_printf("x");
  con->dd_printf(" %s\n", (activated)?"activated":"inactive");
}


/*
 * List of address decoders
 */

cl_decoder_list::cl_decoder_list(t_index alimit, t_index adelta, bool bychip):
  cl_sorted_list(alimit, adelta, "decoder list")
{
  Duplicates= true;
  by_chip= bychip;
}

void *
cl_decoder_list::key_of(void *item)
{
  class cl_address_decoder *d= (class cl_address_decoder *)item;
  if (by_chip)
    return(&(d->chip_begin));
  else
    return(&(d->as_begin));
}

int
cl_decoder_list::compare(void *key1, void *key2)
{
  t_addr k1= *((t_addr*)key1), k2= *((t_addr*)key2);
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
  //FILE *f= c->get_fout();
  /*cmd_fprintf(f,*/c->dd_printf("%s: invalid address ", get_type_name());
  /*cmd_fprintf(f,*/c->dd_printf(mem->addr_format, addr);
  /*cmd_fprintf(f,*/c->dd_printf(" in memory %s.\n", mem->get_name());
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
  //FILE *f= c->get_fout();
  /*cmd_fprintf(f,*/c->dd_printf("%s: access of non-decoded address ", get_type_name());
  /*cmd_fprintf(f,*/c->dd_printf(mem->addr_format, addr);
  /*cmd_fprintf(f,*/c->dd_printf(" in memory %s.\n", mem->get_name());
}

cl_mem_error_registry::cl_mem_error_registry(void)
{
  class cl_error_class *prev = mem_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "memory", prev, ERROR_OFF));
  prev = register_error(new cl_error_class(err_error, "invalid_address", prev));
  prev = register_error(new cl_error_class(err_error, "non_decoded", prev));
}

/* End of mem.cc */
