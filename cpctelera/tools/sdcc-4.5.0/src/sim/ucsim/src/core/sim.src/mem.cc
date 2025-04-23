/*
 * Simulator of microcontrollers (mem.cc)
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <stdarg.h>
#include <string.h>
//#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"

// sim
//#include "simcl.h"

// cmd
//#include "newcmdcl.h"
//#include "cmdutil.h"

// local
#include "memcl.h"


static class cl_mem_error_registry mem_error_registry;
static unsigned int mem_uids= 0;

t_mem def_data;

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
  uid= mem_uids++;
}

cl_memory::~cl_memory(void)
{
  if (addr_format)
    free(addr_format);
  if (data_format)
    free(data_format);
}

bool
cl_memory::is_named(const char *the_name) const
{
  bool b1= cl_base::is_named(the_name);
  if (!the_name || !*the_name)
    return false;
  if (b1 || altname.empty())
    return b1;
  bool b2= altname==the_name;
  return b2;
}

bool
cl_memory::is_inamed(const char *the_name) const
{
  bool b1= cl_base::is_inamed(the_name);
  if (!the_name || !*the_name)
    return false;
  if (b1 || altname.empty())
    return b1;
  bool b2= strcasecmp(altname.c_str(), the_name) == 0;
  return b2;
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
    c+= "L";
  else if (sizeof(t_addr) > sizeof(int))
    c+= "l";
  c+= "x";
  addr_format = strdup(c);

  c.format("%%0%d", width/4+((width%4)?1:0));
  if (sizeof(t_mem) > sizeof(long))
    c+= "L";
  else if (sizeof(t_mem) > sizeof(int))
    c+= "l";
  c+= "x";
  data_format= strdup(c);

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
cl_memory::dump(int smart,
		//t_addr start, t_addr stop,
		class cl_dump_ads *ads,
		int bitnr_high, int bitnr_low,
		int bpl,
		class cl_console_base *con)
{
  if (!con->get_fout())
    return dump_finished;

  if (bpl < 0)
    {
      bpl= 8;
      if (width > 16)
	bpl= 4;
    }

  if (!ads->use_start)
    ads->start= dump_finished;

  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();

  if (ads->start < lva)
    ads->start= lva;
  if (ads->start > hva)
    return dump_finished;

  int lines= -1;
  if (!ads->use_stop)
    {
      ads->stop= hva;
      lines= 10;
    }
  if (ads->stop > hva)
    ads->stop= hva;
  if (ads->stop < lva)
    return dump_finished;

  int i, step;
  if (ads->stop >= ads->start)
    {
      step= +1;
      ads->stop++;
      if (ads->start + bpl > ads->stop)
        bpl= ads->stop - ads->start;
    }
  else
    {
      step= -1;
      ads->stop--;
      if (ads->start - bpl < ads->stop)
        bpl= ads->start - ads->stop;
    }

  long label_width = -1;
  cl_option *o = application->options->get_option("label_width");
  if (o)
    o->get_value(&label_width);
  if (label_width < 0)
    label_width = uc->vars->get_max_name_len();

  bool bitmode = (smart == 2 || (smart && bitnr_high >= 0));

  cl_vars_iterator vi(uc->vars);
  const class cl_var *var = NULL;
  const class cl_var *var_next = NULL;

  int state = 0;

  while ((step>0)?(ads->start < ads->stop):(ads->start > ads->stop))
    {
      if (smart && step > 0 && this == uc->rom && uc->inst_at(ads->start))
        {
          uc->print_disass(ads->start, con, true);
          ads->start += uc->inst_length(ads->start);
          dump_finished = ads->start;
          if (lines > 0 && --lines == 0)
            break;
          continue;
        }

      int n;

      con->dd_color("dump_address");
      if (state == 0)
        con->dd_printf(addr_format, ads->start);
      else
        con->dd_printf("  %-*s", atoi(addr_format+4), "");

      if (smart)
        {
          if (state != 0)
            {
              if (bitnr_high >= 0 || smart == 2)
                con->dd_printf("     ");
            }
          else
            {
              if (bitnr_high >= 0 && bitnr_high == bitnr_low)
                con->dd_printf(".%d   ", bitnr_high);
              else if (bitnr_low > 0 || (bitnr_high > 0 && bitnr_high < width - 1))
                con->dd_printf("[%d:%d]", bitnr_high, bitnr_low);
              else if (smart == 2)
                con->dd_printf("     ");

              state = 1;

              // Find the first var for this location.
              //fprintf(stderr, "Find first var for %s -> %s[0x%04x][%d:%d]\n", this->get_name(), this->get_name(), start, bitnr_high, bitnr_low);
             var = vi.first(this, ads->start);
             while (var)
                {
                  // If _any_ var for this location names bits we output in bitmode
                  // regardless of whether the named bits are in the requested range.
                  if (var->bitnr_high >= 0)
                    bitmode = true;

                  // If the var names bits then we skip it unless all the bits are
                  // contained within the requested range.
                  if (bitnr_high < 0 || var->bitnr_high < 0 || (var->bitnr_high <= bitnr_high && var->bitnr_low >= bitnr_low))
                    break;

                  //fprintf(stderr, "    skip %s -> %s[0x%04x][%d:%d]\n", var->get_name(), var->mem->get_name(), var->addr, var->bitnr_high, var->bitnr_low);
                  var = vi.next();
                }
            }

          //if (var_i < uc->vars->by_addr.count && var)
            //fprintf(stderr, "state = %d, %s -> %s[0x%04x][%d:%d]\n", state, var->get_name(), var->mem->get_name(), var->addr, var->bitnr_high, var->bitnr_low);

          if (var &&
              (var->bitnr_high < 0 ||
	       (state < 2 && ((var->bitnr_high == bitnr_high && var->bitnr_low == bitnr_low) ||
                              (bitnr_high < 0 && var->bitnr_high == width - 1 && var->bitnr_low == 0))) ||
	       (state == 2 && (bitnr_high < 0 || (var->bitnr_high <= bitnr_high && var->bitnr_low >= bitnr_low)))))
            {
              con->dd_color("dump_label");

              // If we wanted specific bits but the var is just a label we need to qualify it.
              if (bitnr_high >= 0 && var->bitnr_high < 0)
                {
                  if (bitnr_high == bitnr_low)
                    con->dd_printf(" %s.%d:   %*s", var->get_name(), bitnr_high, label_width - strlen(var->get_name()), "");
                  else
                    con->dd_printf(" %s[%d:%d]:%*s", var->get_name(), bitnr_high, bitnr_low, label_width - strlen(var->get_name()), "");
                }
              else
                con->dd_printf(" %s:%*s", var->get_name(), label_width - strlen(var->get_name()) + (smart == 2 ? 5 : 0), "");

              // Find the next relevant var.
              while ((var_next = vi.next()))
                {
                  // If _any_ var for this location names bits we output in bitmode
                  // regardless of whether the named bits are in the requested range.
                  if (var_next->bitnr_high >= 0)
                    bitmode = true;

                  // If the var names bits then we skip it unless all the bits are
                  // contained within the requested range.
                  if (bitnr_high < 0 || var_next->bitnr_high < 0 || (var_next->bitnr_high <= bitnr_high && var_next->bitnr_low >= bitnr_low))
                    break;

                  //fprintf(stderr, "    skip next %s -> %s[0x%04x][%d:%d]\n", var_next->get_name(), var_next->mem->get_name(), var_next->addr, var_next->bitnr_high, var_next->bitnr_low);
                }
              //if (var_i < uc->vars->by_addr.count && var_next)
                  //fprintf(stderr, "    next  is %s -> %s[0x%04x][%d:%d]\n", var_next->get_name(), var_next->mem->get_name(), var_next->addr, var_next->bitnr_high, var_next->bitnr_low);

              if (var_next)
                {
                  // If it aliases the previous we do not need to output data now.
                  if ((var_next->bitnr_high == var->bitnr_high && var_next->bitnr_low == var->bitnr_low) ||
                      (state == 1 &&
                       (var->bitnr_high < 0 &&
                        ((var_next->bitnr_high == width - 1 && var_next->bitnr_low == 0) ||
                         (bitnr_high >= 0 && var_next->bitnr_high == bitnr_high && var_next->bitnr_low >= bitnr_low)))))
                    {
                      con->dd_printf("\n");
                      if (lines > 0)
                        lines--;
                      var = var_next;
                      var_next = NULL;
                      continue;
                    }
                }
            }
          else
            {
              var_next = var;
              con->dd_printf(" %-*s %s", label_width, "", (smart == 2 ? "     " : ""));
            }

          con->dd_color("dump_number");

          if (bitmode)
            {
              int b_high, b_low;

              if (var && state == 2 && var->bitnr_high >= 0)
                b_high = var->bitnr_high, b_low = var->bitnr_low;
              else if (bitnr_high >= 0)
                b_high = bitnr_high, b_low = bitnr_low;
              else
                b_high = width - 1, b_low = 0;

              con->dd_printf(" ");

              /*t_mem*/u64_t m= read(ads->start);

              int i;
              con->dd_printf("0b");
              for (i= width - 1; i > b_high; i--)
                con->dd_printf("-");
              for (; i >= b_low; i--)
                con->dd_printf("%c", (m & (1U << i)) ? '1' : '0');
              for (; i >= 0; i--)
                con->dd_printf("-");

              int nbits = b_high - b_low + 1;

              m = (m >> b_low) & (((u64_t)1U << nbits) - 1);

              con->dd_printf(" 0x%0*x", (nbits > 16 ? 8 : (nbits > 8 ? 4 : 2)), m);

              con->dd_color("dump_char");
              con->dd_printf(" '");
	      // FIXME: endian assumption: low byte first - should be a uc flag or option?
              for (int ii= 0; ii <= (nbits - 1) - ((nbits - 1) % 8); ii += 8)
                con->dd_printf("%c", (isprint((m >> ii) & 255) ? (m >> ii) & 255 : '.'));
              con->dd_printf("'");
              con->dd_color("dump_number");

              con->dd_printf(" %*u", (nbits > 16 ? 10 : (nbits > 8 ? 5 : 3)), m);

              if (nbits == width && (m & (1U << (nbits - 1))))
                con->dd_printf(" (%*d)", (nbits > 16 ? 10 : (nbits > 8 ? 5 : 3)), 0 - ((1U << nbits) - m));

              con->dd_printf("\n");
              if (lines > 0)
                lines--;

              state = 2;

              // Only advance if there is no more to say about this location.
              if (var_next)
                var = var_next;
              else
                {
                  ads->start += step;
                  dump_finished= ads->start;

                  if (lines == 0)
                    break;

                  bitmode = (smart == 2 || (smart && bitnr_high >= 0));
                  state = 0;
                }

              continue;
            }

            // Not bit-formatted so drop through to normal output.
        }

      state = 2;
      con->dd_color("dump_number");

      for (n= 0;
           (n < bpl) &&
             (ads->start+n*step >= lva) &&
             (ads->start+n*step <= hva) &&
             (ads->start+n*step != ads->stop);
           n++)
        {
          if (smart && n)
            {
              if (uc->inst_at(ads->start+n*step) || (var = vi.first(this, ads->start+n*step)))
                break;
            }
          con->dd_printf(" ");
          con->dd_printf(data_format, read(ads->start+n*step));
        }
      con->dd_printf("%-*s", (bpl - n) * (width/4 + ((width%4)?1:0) + 1) + 1, " ");

      con->dd_color("dump_char");
      for (i= 0; i < n &&
             ads->start+i*step >= lva &&
             ads->start+i*step <= hva &&
             ads->start+i*step != ads->stop;
           i++)
        {
          long c= read(ads->start+i*step);
	  // FIXME: endian assumption: low byte first - should be a uc flag or option?
          for (int ii= 0; ii <= (width - 1) - ((width - 1) % 8); ii += 8)
            con->dd_printf("%c", (isprint((c >> ii) & 255) ? (c >> ii) & 255 : '.'));
        }
      con->dd_printf("\n");

      ads->start+= n*step;
      dump_finished= ads->start;
      state = 0;
      if (lines > 0 && --lines == 0)
        break;
    }

  if (dump_finished >= hva)
    dump_finished= lva;

  return(dump_finished);
}

t_addr
cl_memory::dump_s(//t_addr start, t_addr stop,
		  class cl_dump_ads *ads,
		  int bpl,
		  class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  class cl_f *f= con->get_fout();

  t_addr a= ads->start;
  t_mem d= read(a);
  char last= '\n';
  con->dd_printf("%s", con->get_color_ansiseq("dump_char").c_str());
  while ((d != 0) &&
	 (a <= hva))
    {
      char c= d;
      int i= d & 0xff;
      if (a >= lva)
	{
	  switch (c)
	    { // ' " ? \ a b f n r t v
	    case '\'': f->write("\\\'", 2); break;
	    case '\"': f->write("\\\"", 2); break;
	    case '\?': f->write("\\\?", 2); break;
	    case '\\': f->write("\\\\", 2); break;
	    case '\a': f->write("\\a", 2); break;
	    case '\b': f->write("\\b", 2); break;
	    case '\f': f->write("\\f", 2); break;
	    case '\n': f->write("\\n", 2); break;
	    case '\r': f->write("\\r", 2); break;
	    case '\t': f->write("\\t", 2); break;
	    case '\v': f->write("\\v", 2); break;
	    default:
	      if (isprint(i))
		f->write(&c, 1);
	      else
		{
		  chars s = chars("", "\\%03o", i);
		  f->write(s.c_str(), s.len());
		}
	    }
	  last= c;
	}
      d= read(++a);
    }
  if (last != '\n')
    f->write_str("\n");

  dump_finished= a;
  if (dump_finished >= hva)
    dump_finished= lva;

  return dump_finished;
}

t_addr
cl_memory::dump_b(//t_addr start, t_addr stop,
		  class cl_dump_ads *ads,
		  int bpl,
		  class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  class cl_f *f= con->get_fout();

  if (!ads->use_stop)
    ads->stop= ads->start + 10 * bpl - 1;

  t_addr a= ads->start;
  t_mem d= read(a);
  while ((a <= ads->stop) &&
	 (a <= hva))
    {
      char c= d;
      if (a >= lva)
	{
	  f->write(&c, 1);
	}
      d= read(++a);
    }

  dump_finished= a;
  if (dump_finished >= hva)
    dump_finished= lva;

  return dump_finished;
}

t_addr
cl_memory::dump_i(//t_addr start, t_addr stop,
		  class cl_dump_ads *ads,
		  int bpl,
		  class cl_console_base *con)
{
  t_addr lva= lowest_valid_address();
  t_addr hva= highest_valid_address();
  unsigned int sum;
  t_addr start_line;
  class cl_f *f= con->get_fout();
  
  if (!ads->use_stop)
    ads->stop= ads->start + 10 * bpl - 1;

  if (ads->start < lva)
    ads->start= lva;
  if (ads->stop > hva)
    ads->stop= hva;

  if (ads->start < lva)
    ads->start= lva;
  if (ads->start > hva)
    return dump_finished;
  if (ads->stop > hva)
    ads->stop= hva;
  if (ads->stop < lva)
    return dump_finished;
  
  if (ads->start > ads->stop)
    return dump_finished= ads->stop;
  if (bpl > 32)
    bpl= 32;
  t_addr a= ads->start;
  sum= 0;
  start_line= a;
  while (a <= ads->stop)
    {
      a++;
      if (((a % bpl) == 0) ||
	  (a > ads->stop))
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
	      unsigned int i;
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

  dump_finished= a;
  if (dump_finished >= hva)
    dump_finished= lva;

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
cl_memory::print_info(const char *pre, class cl_console_base *con)
{
  if (!hidden)
    {
      con->dd_printf("%s0x%06x-0x%06x %8d %s (%d,%s,%s)\n", pre,
		     AU(get_start_address()),
		     AU(highest_valid_address()),
		     AU(get_size()),
		     get_name(),
		     width, data_format, addr_format);
    }
}


/*
 *                                                             Memory operators
 */

cl_memory_operator::cl_memory_operator(class cl_memory_cell *acell):
  cl_base()
{
  cell= acell;
  if (cell)
    {
      mask= cell->mask;
    }
  else
    {
      mask= ~0;
    }
}

t_mem
cl_memory_operator::read(void)
{
  if (cell)
    return(cell->get());
  return 0;
}

t_mem
cl_memory_operator::write(t_mem val)
{
  /*if (cell)
    return(cell->set(val));*/
  return val;
}


/* Memory operator for bank switcher */

cl_bank_switcher_operator::cl_bank_switcher_operator(class cl_memory_cell *acell,
						     class cl_banker *the_banker):
  cl_memory_operator(acell)
{
  banker= the_banker;
  set_name("bank_switcher");
}

t_mem
cl_bank_switcher_operator::write(t_mem val)
{
  if (cell)
    cell->set(val);
  banker->activate(NULL);
  if (cell)
    return cell->get();
  return 0;  
}


/* Memory operator for hw callbacks */

cl_hw_operator::cl_hw_operator(class cl_memory_cell *acell,
			       class cl_hw *ahw):
  cl_memory_operator(acell)
{
  hw= ahw;
  set_name(chars("hw:")+hw->get_name());
}


t_mem
cl_hw_operator::read(void)
{
  t_mem d1= 0;

  if (hw)
    {
      bool act= hw->active;
      hw->active = true;
      d1= hw->read(cell);
      hw->active = act;
      return d1;
    }
  
  return cl_memory_operator::read();
}

t_mem
cl_hw_operator::read(enum hw_cath skip)
{
  t_mem d1= 0;

  if (hw && (hw->category != skip))
    {
      bool act= hw->active;
      hw->active= true;
      d1= hw->read(cell);
      hw->active= act;
      return d1;
    }

  return cl_memory_operator::read();
}

t_mem
cl_hw_operator::write(t_mem val)
{
  if (hw)
    {
      bool act= hw->active;
      hw->active= true;
      hw->write(cell, &val);
      hw->active= act;
      return val;
    }

  return val;
}


/* Write event break on cell */

cl_write_operator::cl_write_operator(class cl_memory_cell *acell,
				     class cl_uc *auc, class cl_brk *the_bp):
  cl_event_break_operator(acell, auc, the_bp)
{
  uc= auc;
  bp= the_bp;
  set_name("write_event");
}

t_mem
cl_write_operator::write(t_mem val)
{
  if (bp->do_hit())
    uc->events->add(bp);
  /*if (cell)
    return(cell->set(val));*/
  return val;
}


/* Read event break on cell */

cl_read_operator::cl_read_operator(class cl_memory_cell *acell,
				   class cl_uc *auc, class cl_brk *the_bp):
  cl_event_break_operator(acell, auc, the_bp)
{
  uc= auc;
  bp= the_bp;
  set_name("read_event");
}

t_mem
cl_read_operator::read(void)
{
  if (bp->do_hit())
    uc->events->add(bp);
  if (cell)
    return(cell->get());
  return 0;
}


/*
 *                                                                  Cell data
 */

// 8 bit cell;

t_mem
cl_cell8::d()
{
  return *((u8_t*)data);
  return data?(*((u8_t*)data)):0;
}

void
cl_cell8::d(t_mem v)
{
  *((u8_t*)data)=(u8_t)v;
  //data?(*((u8_t*)data)=(u8_t)v):0;
}

void
cl_cell8::dl(t_mem v)
{
  *((u8_t*)data)=(u8_t)v;
  //data?(*((u8_t*)data)=(u8_t)v):0;
}

// 8 bit cell for bit spaces

t_mem
cl_bit_cell8::d()
{
  //if (!data) return 0;
  u8_t x= *((u8_t*)data);
  x&= mask;
  return x?1:0;
}

void
cl_bit_cell8::d(t_mem v)
{
  //if (!data) return;
  if (v)
    *((u8_t*)data) |= (u8_t)mask;
  else
    *((u8_t*)data) &= ~((u8_t)mask);
}


// 16 bit cell;

t_mem
cl_cell16::d()
{
  return data?(*((u16_t*)data)):0;
}

void
cl_cell16::d(t_mem v)
{
  data?(*((u16_t*)data)=(u16_t)v):0;
}

void
cl_cell16::dl(t_mem v)
{
  data?(*((u16_t*)data)=(u16_t)v):0;
}

// 16 bit cell for bit spaces

t_mem
cl_bit_cell16::d()
{
  if (!data)
    return 0;
  u16_t x= *((u16_t*)data);
  x&= mask;
  return x?1:0;
}

void
cl_bit_cell16::d(t_mem v)
{
  if (!data)
    return;
  if (v)
    *((u16_t*)data) |= (u16_t)mask;
  else
    *((u16_t*)data) &= ~((u16_t)mask);
}

// 32 bit cell;

t_mem
cl_cell32::d()
{
  return *((u32_t*)data);
  return data?(*((u32_t*)data)):0;
}

void
cl_cell32::d(t_mem v)
{
  *((u32_t*)data)=(u32_t)v;
  //data?(*((u32_t*)data)=(u32_t)v):0;
}

void
cl_cell32::dl(t_mem v)
{
  *((u32_t*)data)=(u32_t)v;
  //data?(*((u32_t*)data)=(u32_t)v):0;
}

// 32 bit cell for bit spaces

t_mem
cl_bit_cell32::d()
{
  //if (!data) return 0;
  u32_t x= *((u32_t*)data);
  x&= mask;
  return x?1:0;
}

void
cl_bit_cell32::d(t_mem v)
{
  //if (!data) return;
  if (v)
    *((u32_t*)data) |= (u32_t)mask;
  else
    *((u32_t*)data) &= ~((u32_t)mask);
}


/*
 *                                                                  Memory cell
 */

cl_memory_cell::cl_memory_cell()
{
  data= 0;
  flags= CELL_NON_DECODED;
  width= 8;
  //def_data= 0;
  ops= NULL;
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

cl_memory_cell::cl_memory_cell(uchar awidth)
{
  data= 0;
  flags= CELL_NON_DECODED;
  width= awidth;
  //def_data= 0;
  ops= NULL;
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
  if (ops)
    {
      /*int i;
      for (i=0; ops[i]; i++)
      delete ops[i];*/
      free(ops);
    }
}

int
cl_memory_cell::init(void)
{
  data= &def_data;
  return(0);
}

void
cl_memory_cell::set_width(uchar awidth)
{
  width= awidth;
  mask= 1;
  int w= width;
  for (--w; w; w--)
    {
      mask<<= 1;
      mask|= 1;
    }
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
      data= &def_data;
      flags|= CELL_NON_DECODED;
    }
}

void
cl_memory_cell::decode(class cl_memory_chip *chip, t_addr addr)
{
  data= chip->get_slot(addr);
  if (!data)
    {
      data= &def_data;
      flags|= CELL_NON_DECODED;
    }
  else
    flags&= ~(CELL_NON_DECODED);
}

void
cl_memory_cell::decode(void *data_ptr)
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
cl_memory_cell::decode(void *data_ptr, t_mem bit_mask)
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
  if (ops && ops[0])
    {
      t_mem r= 0;
      for (int i=0; ops[i]; i++)
	r= ops[i]->read();
      return r;
    }
  return d();
}

t_mem
cl_memory_cell::read(enum hw_cath skip)
{
#ifdef STATISTIC
  nuof_reads++;
#endif
  if (ops && ops[0])
    {
      t_mem r;
      for (int i=0; ops[i]; i++)
	r= ops[i]->read(skip);
      return r;
    }
  return d();
}

t_mem
cl_memory_cell::get(void)
{
  return d();
}

t_mem
cl_memory_cell::write(t_mem val)
{
#ifdef STATISTIC
  nuof_writes++;
#endif
  if (ops && ops[0])
    {
      for (int i=0; ops[i]; i++)
	val= ops[i]->write(val);
    }
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
    d(val & mask);
  return d();
}

t_mem
cl_memory_cell::download(t_mem val)
{
  if (width == 1)
    dl(val);
  else
    dl(val & mask);
  return d();
}

// Number of operator, NULL termination is not included
int
cl_memory_cell::nuof_ops(void)
{
  int i;
  if (!ops)
    return 0;
  for (i=0; ops[i]!=NULL; i++) ;
  return i;
}

void
cl_memory_cell::append_operator(class cl_memory_operator *op)
{
  class cl_memory_operator **op_p;
  int i= nuof_ops()+2;
  if (!ops)
    {
      op_p= (cl_memory_operator**)malloc(sizeof(void*) * i);
      op_p[0]= NULL;
    }
  else
    op_p= (cl_memory_operator**)realloc(ops, sizeof(void*) * i);
  ops= op_p;
  for (i=0; ops[i]!=NULL; i++) ;
  ops[i]= op;
  ops[i+1]= NULL;
}

void
cl_memory_cell::prepend_operator(class cl_memory_operator *op)
{
  if (!op)
    return;
  int i= nuof_ops() + 2;
  class cl_memory_operator **p=
    (cl_memory_operator**)malloc(sizeof(void*) * i);
  p[0]= op;
  for (i=0; ops && ops[i]!=NULL; i++)
    p[i+1]= ops[i];
  p[i+1]= NULL;
  if (ops)
    free(ops);
  ops= p;
}

void
cl_memory_cell::remove_operator(class cl_memory_operator *op)
{
  int src, dst;
  for (src=dst=0; ops && ops[src]!=NULL; src++)
    {
      if (ops[src]!=op)
	{
	  if (src!=dst)
	    ops[dst]= ops[src];
	  dst++;
	}
    }
  if (ops)
    {
      ops[dst]= NULL;
      if (dst == 0)
	free(ops), ops= NULL;
    }
}

void
cl_memory_cell::del_operator(class cl_brk *brk)
{
  class cl_memory_operator *old;
  int src, dst;
  old= NULL;
  for (src=dst=0; ops && ops[src]!=NULL; src++)
    {
      if (!(ops[src]->match(brk)))
	{
	  if (src!=dst)
	    ops[dst]= ops[src];
	  dst++;
	}
      else
	old= ops[src];
    }
  ops[dst]= NULL;
  if (old)
    delete old;
  if (dst==0)
    free(ops), ops= NULL;
}

void 	 
cl_memory_cell::del_operator(class cl_hw *hw)
{
  class cl_memory_operator *old;
  int src, dst;
  old= NULL;
  for (src=dst=0; ops && ops[src]!=NULL; src++)
    {
      if (!(ops[src]->match(hw)))
	{
	  if (src!=dst)
	    ops[dst]= ops[src];
	  dst++;
	}
      else
	old= ops[src];
    }
  ops[dst]= NULL;
  if (old)
    delete old;
  if (dst==0)
    free(ops), ops= NULL;
}

class cl_banker *
cl_memory_cell::get_banker(void)
{
  class cl_banker *b= NULL;
  for (int i=0; ops && ops[i]; i++)
    if ((b= ops[i]->get_banker())!=NULL)
      return b;
  return NULL;
}

void
cl_memory_cell::set_brk(class cl_uc *uc, class cl_brk *brk)
{
  class cl_memory_operator *op;

  if (!brk) return;
  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
      //e= 'W';
      op= new cl_write_operator(this, uc, brk);
      break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      //e= 'R';
      op= new cl_read_operator(this, uc, brk);
      break;
    case brkNONE:
      set_flag(CELL_FETCH_BRK, true);
      return;
      break;
    default:
      //e= '.';
      op= 0;
      break;
    }
  if (op)
    append_operator(op);
}

void
cl_memory_cell::del_brk(class cl_brk *brk)
{
  if (!brk) return;
  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      del_operator(brk);
      break;
    case brkNONE:
      set_flag(CELL_FETCH_BRK, false);
      break;
    default:
      break;
    }
}

class cl_memory_cell *
cl_memory_cell::add_hw(class cl_hw *hw)
{
  class cl_hw_operator *o= new cl_hw_operator(this, hw);
  append_operator(o);
  return(this);
}

void 	 
cl_memory_cell::remove_hw(class cl_hw *hw) 	 
{ 	 
  del_operator(hw); 	 
}

class cl_event_handler *
cl_memory_cell::get_event_handler(void)
{
  return(0);
}

void
cl_memory_cell::print_info(const char *pre, class cl_console_base *con)
{
  con->dd_printf("%sFlags:", pre);
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
cl_memory_cell::print_operators(const char *pre, class cl_console_base *con)
{
  for (int i=0; ops && ops[i]!=NULL; i++)
    {
      if (con) con->dd_printf("%s [%d] %s\n", pre, i, (ops[i])->get_name("?"));
      else printf("%s [%d] %s\n", pre, i, (ops[i])->get_name("?"));
    }
}


/*
 * Dummy cell for non-existent addresses
 */

t_mem
cl_dummy_cell::d()
{
  return data?(*((u32_t*)data)):0;
}

void
cl_dummy_cell::d(t_mem v)
{
  *((u32_t*)data)= (u32_t)v;
}

t_mem
cl_dummy_cell::write(t_mem val)
{
#ifdef STATISTIC
  nuof_writes++;
#endif
  *((u32_t*)data)= urnd() & mask;
  return(*((u32_t*)data));
}

t_mem
cl_dummy_cell::set(t_mem val)
{
  *((u32_t*)data)= urnd() & mask;
  return(*((u32_t*)data));
}


/*
 *                                                                Address space
 */

cl_address_space::cl_address_space(const char *id,
				   t_addr astart, t_addr asize, int awidth):
  cl_memory(id, asize, awidth)
{
  start_address= astart;
  decoders= new cl_decoder_list(2, 2, false);
  cella= (class cl_memory_cell *)malloc(size * sizeof(class cl_memory_cell));
  dummy= new cl_dummy_cell(awidth);
  dummy->init();
}

int
cl_address_space::init(void)
{
  cl_memory::init();
  class cl_memory_cell *cell= cell_template();
  unsigned int i, s= sizeof(class cl_cell32);
  //cell->as= this;
  u8_t *p1= (u8_t*)cella;
  cell->init();
  for (i= 0; i < size; i++)
    {
      memcpy(p1, (void*)cell, s);
      p1+= s;
    }
  
  return 0;
}

static class cl_bit_cell8 bc8_tmpl;
static class cl_cell8 c8_tmpl;
static class cl_cell16 c16_tmpl;
static class cl_cell32 c32_tmpl;

class cl_memory_cell *
cl_address_space::cell_template()
{
  class cl_memory_cell *cell= &c32_tmpl;
  if (width == 1)
    cell= &bc8_tmpl;
  else if (width <= 8)
    cell= &c8_tmpl;
  else if (width <= 16)
    cell= &c16_tmpl;
  if (cell) cell->set_width(width);
  return cell;
}

cl_address_space::~cl_address_space(void)
{
  delete decoders;
  unsigned int i;
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
/*
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
*/


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
  unsigned int i;

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
	(class cl_address_decoder *)(decoders->object_at(i));
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
	  // decoder should shrink
	  D("    Should shrink\n");
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
	      D("    Shrunk to 0x%lx-0x%lx -> %s[0x%lx]\n",
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
    {
      //uc->sim->/*app->*/mem_cell_changed(this, addr);//FIXME
    }
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
  /*
  class cl_memory_operator *op= 0;

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
      //e= 'W';
      op= new cl_write_operator(cell, uc, brk);
      break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      //e= 'R';
      op= new cl_read_operator(cell, uc, brk);
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
  */
  cell->set_brk(uc, brk);
}

void
cl_address_space::del_brk(t_addr addr, class cl_brk *brk)
{
  t_addr idx= addr-start_address;
  if (idx >= size ||
      addr < start_address)
    return;
  class cl_memory_cell *cell= &cella[idx];
  /*
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
  */
  cell->del_brk(brk);
}

void
cl_address_space::print_info(const char *pre, class cl_console_base *con)
{
  if (!hidden)
    {
      con->dd_printf("%s0x%06x-0x%06x %8d %s (%d,%s,%s)\n", pre,
		     AU(get_start_address()),
		     AU(highest_valid_address()),
		     AU(get_size()),
		     get_name(),
		     width, data_format, addr_format);
    }
}


/*
 * List of address spaces
 */

cl_memory_list::cl_memory_list(class cl_uc *the_uc, const char *name):
  cl_list(2, 2, name)
{
  uc= the_uc;
}

t_index
cl_memory_list::add(class cl_memory *mem)
{
  t_index ret= cl_list::add(mem);
  mem->set_uc(uc);
  if (uc && mem->is_address_space())
    {
      class cl_event_address_space_added e((cl_address_space *)mem);
      uc->handle_event(e);
    }
  return(ret);
}


/*
 *                                                                  Memory chip
 */

cl_chip_data::cl_chip_data(const char *id, t_addr asize, int awidth):
  cl_memory(id, asize, awidth)
{
}

cl_memory_chip::cl_memory_chip(const char *id,
			       int asize,
			       int awidth,
			       int initial):
  cl_chip_data(id, asize, awidth)
{
  if (awidth > 32)
    width= 32;
  if (width <= 8)
    bwidth= 1;
  else if (width <= 16)
    bwidth= 2;
  else
    bwidth= 4;
  alloc_size= size * sizeof(t_mem);
  array= (t_mem *)malloc(alloc_size);
  init_value= initial;
  array_is_mine= true;
}

cl_memory_chip::cl_memory_chip(const char *id,
			       int asize,
			       int awidth,
			       void *aarray, int arrsize):
  cl_chip_data(id, asize, awidth)
{
if (awidth > 32)
    width= 32;
  if (width <= 8)
    bwidth= 1;
  else if (width <= 16)
    bwidth= 2;
  else
    bwidth= 4;
  alloc_size= arrsize;
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
  t_addr i;
  if (array_is_mine)
    {
      for (i= 0; i < size; i++)
	set(i,
	    (init_value<0)?urnd():(init_value)
	    );
    }
  return(0);
}


void *
cl_memory_chip::get_slot(t_addr addr)
{
  u8_t *a= (u8_t*)array;
  if (!array ||
      size <= addr)
    return(0);
  return(a+(addr*bwidth));
}

bool
cl_memory_chip::is_slot(void *data_ptr, t_addr *addr_of)
{
  u8_t *p= (u8_t *)data_ptr;
  u8_t *a= (u8_t *)array;
  if (p < &(a[0]))
    return false;
  if (p > &(a[alloc_size-1]))
    return false;
  t_addr i= p - a;
  if (width <= 8)
    /*i*/;
  if (width <= 16)
    i= i/2;
  if (width <= 32)
    i= i/4;
  if (addr_of)
    *addr_of= i;
  return true;
}
/*
t_mem
cl_memory_chip::get(t_addr addr)
{
  if (!array ||
      size <= addr)
    return(0);
  return(array[addr]);
}
*/
/*
void
cl_memory_chip::set(t_addr addr, t_mem val)
{
  if (!array ||
      size <= addr)
    return;
  array[addr]= val & data_mask;
}
*/
void
cl_memory_chip::print_info(const char *pre, class cl_console_base *con)
{
  if (!hidden)
    {
      //con->dd_printf(pre0);
      con->dd_printf("%s0x%06x-0x%06x %8d %s (%d,%s,%s)\n", pre,
		     AU(get_start_address()),
		     AU(highest_valid_address()),
		     AU(get_size()),
		     get_name(),
		     width, data_format, addr_format);
    }
}


// Chip with 1 byte slots

cl_chip8::cl_chip8(const char *id, int asize, int awidth, int initial):
  cl_memory_chip(id, asize, awidth, initial)
{}

cl_chip8::cl_chip8(const char *id, int asize, int awidth, void *aarray, int arrsize):
  cl_memory_chip(id, asize, awidth, aarray, arrsize)
{}

t_mem
cl_chip8::d(t_addr addr)
{
  if (!array ||
      size <= addr)
    return 0;
  return (((u8_t*)array)[addr]) & data_mask;
}

void
cl_chip8::d(t_addr addr, t_mem v)
{
  if (!array ||
      size <= addr)
    return;
  v&= data_mask;
  ((u8_t*)array)[addr]= v;
}


// Chip with 2 byte slots

cl_chip16::cl_chip16(const char *id, int asize, int awidth, int initial):
  cl_memory_chip(id, asize, awidth, initial)
{}

cl_chip16::cl_chip16(const char *id, int asize, int awidth, void *aarray, int arrsize):
  cl_memory_chip(id, asize, awidth, aarray, arrsize)
{}

t_mem
cl_chip16::d(t_addr addr)
{
  if (!array ||
      size <= addr)
    return 0;
  return (((u16_t*)array)[addr]) & data_mask;
}

void
cl_chip16::d(t_addr addr, t_mem v)
{
  if (!array ||
      size <= addr)
    return;
  v&= data_mask;
  ((u16_t*)array)[addr]= v;
}


// Chip with 4 byte slots

cl_chip32::cl_chip32(const char *id, int asize, int awidth, int initial):
  cl_memory_chip(id, asize, awidth, initial)
{}

cl_chip32::cl_chip32(const char *id, int asize, int awidth, void *aarray, int arrsize):
  cl_memory_chip(id, asize, awidth, aarray, arrsize)
{}

t_mem
cl_chip32::d(t_addr addr)
{
  if (!array ||
      size <= addr)
    return 0;
  return (((u32_t*)array)[addr]) & data_mask;
}

void
cl_chip32::d(t_addr addr, t_mem v)
{
  if (!array ||
      size <= addr)
    return;
  v&= data_mask;
  ((u32_t*)array)[addr]= v;
}

class cl_memory_chip *
new_chip(const char *id, int asize, int awidth, int initial)
{
  class cl_memory_chip *c= 0;
  
  if (awidth < 1)
    awidth= 1;
  if (awidth > 32)
    awidth= 32;

  if (awidth <= 8)
    c= new cl_chip8(id, asize, awidth, initial);
  else if (awidth <= 16)
    c= new cl_chip16(id, asize, awidth, initial);
  else
    c= new cl_chip32(id, asize, awidth, initial);

  if (c)
    c->init();

  return c;
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
cl_address_decoder::print_info(const char *pre, class cl_console_base *con)
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
  op1= NULL;
  op2= NULL;
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
  op1= NULL;
  op2= NULL;
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
	new cl_bank_switcher_operator(c, this);
      c->prepend_operator(o);
      op1= o;
    }
  if (banker2_as &&
      banker2_mask)
    {
      c= banker2_as->get_cell(banker2_addr);
      if (c)
	{
	  class cl_bank_switcher_operator *o=
	    new cl_bank_switcher_operator(c, this);
	  c->prepend_operator(o);
	  op2= o;
	}
    }
  return 0;
}

cl_banker::~cl_banker()
{
  int i;
  class cl_memory_cell *c;
  if (banker_as)
    {
      c= banker_as->get_cell(banker_addr);
      if (c)
	{
	  if (op1) c->remove_operator(op1);
	  if (op2) c->remove_operator(op2);
	}
    }
  if (banker2_as)
    {
      c= banker2_as->get_cell(banker2_addr);
      if (c)
	{
	  if (op1) c->remove_operator(op1);
	  if (op2) c->remove_operator(op2);
	}
    }
    
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
  void *data;
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
  void *data;
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
cl_banker::uses_chip(class cl_memory *chip)
{
  int i;
  for (i= 0; i < nuof_banks; i++)
    {
      if (banks[i])
	{
	  class cl_address_decoder *ad=
	    (class cl_address_decoder *)(banks[i]);
	  if (ad->memchip == chip)
	    return true;
	}
    }
  return false;
}

void
cl_banker::print_info(const char *pre, class cl_console_base *con)
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
      void *slot= memchip->get_slot(ca);
      cl_memory_cell *c= address_space->get_cell(asa);
      c->decode(slot, m);
      b++;
      m<<= 1;
    }
  return activated= true;
}

void
cl_bander::print_info(const char *pre, class cl_console_base *con)
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

const void *
cl_decoder_list::key_of(const void *item) const
{
  const class cl_address_decoder *d= (const class cl_address_decoder *)item;
  if (by_chip)
    return(&(d->chip_begin));
  else
    return(&(d->as_begin));
}

int
cl_decoder_list::compare(const void *key1, const void *key2)
{
  t_addr k1= *(const t_addr *)key1;
  t_addr k2= *(const t_addr *)key2;

  if (k1 == k2)
    return(0);
  else if (k1 < k2)
    return(-1);
  return(1);
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
  c->dd_printf("%s: invalid address ", (char*)get_type_name());
  c->dd_printf(mem->addr_format, addr);
  c->dd_printf(" in memory %s.\n", (char*)(mem->get_name()));
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
  c->dd_printf("%s: access of non-decoded address ", (char*)get_type_name());
  c->dd_printf(mem->addr_format, addr);
  c->dd_printf(" in memory %s.\n", (char*)(mem->get_name()));
}

cl_mem_error_registry::cl_mem_error_registry(void)
{
  class cl_error_class *prev = mem_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "memory", prev, ERROR_OFF));
  prev = register_error(new cl_error_class(err_error, "invalid_address", prev));
  prev = register_error(new cl_error_class(err_error, "non_decoded", prev));
}

/* End of sim.src/mem.cc */
