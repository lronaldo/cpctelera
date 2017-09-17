/*
 * Simulator of microcontrollers (stm8.src/timer.cc)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
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

/* $Id: timer.cc 677 2017-03-07 08:16:02Z drdani $ */

#include "itsrccl.h"

#include "clkcl.h"
#include "timercl.h"


enum tim_cr1_bits {
  cen	= 0x01,
  udis	= 0x02,
  urs	= 0x04,
  opm	= 0x08,
  dir	= 0x10,
  cms	= 0x60,
  cms0	= 0x00, // edge aligned mode
  cms1	= 0x20, // center aligned 1 (irq during downcount)
  cms2	= 0x40, // center aligned 2 (irq during upcount)
  cms3	= 0x60, // center aligned 3 (irq in both counting dir)
  arpe	= 0x80
};

enum tim_sr1_bits {
  uif	= 0x01
};

enum tim_ier_bits {
  uie	= 0x01
};

enum tim_egr_bits {
  ug	= 0x01
};

cl_tim::cl_tim(class cl_uc *auc, int aid, t_addr abase):
  cl_hw(auc, HW_TIMER, aid, "tim")
{
  base= abase;
  int i;
  for (i= 0; i<32+6; i++)
    regs[i]= 0;
  memset(&idx, 0xff, sizeof(idx));
}

int
cl_tim::init(void)
{
  int i;
  chars s("tim");
  s.append("%d", id);
  set_name(s);
  id_string= strdup(s);
  cl_hw::init();
  for (i= 0; i < 32+6; i++)
    {
      regs[i]= register_cell(uc->rom, base+i);
    }

  switch (id)
    {
    case 2:
      bits= 16; mask= 0xffff;      
      break;
    case 3:
      bits= 16; mask= 0xffff;      
      break;
    case 5:
      bits= 16; mask= 0xffff;      
      break;
    case 4:
      bits= 8; mask= 0xff;      
      break;
    case 6:
      bits= 8; mask= 0xff;      
      break;
    default: // 1
      bits= 16; mask= 0xffff;      
      break;
    }
  pbits= 16;
  bidir= true;
  clk_enabled= false;
  
  return 0;
}

int
cl_tim::tick(int cycles)
{
  if (!on ||
      !clk_enabled)
    return resGO;
  
  while (cycles--)
    {
      // count prescaler
      if (prescaler_cnt)
	prescaler_cnt--;
      if (prescaler_cnt == 0)
	{
	  prescaler_cnt= calc_prescaler() - 1;
	  // count
	  if (regs[idx.cr1]->get() & cen)
	    {
	      count();
	    }
	}
    }
  
  return resGO;
}

void
cl_tim::reset(void)
{
  int i;
  
  cnt= 0;
  prescaler_cnt= 0;
  prescaler_preload= 0;

  for (i= 0; i<32+6; i++)
    regs[i]->set(0);
  if (bits > 8)
    regs[idx.arrh]->set(0xff);
  regs[idx.arrl]->set(0xff);

  update_event();
  regs[idx.sr1]->set_bit0(uif);
}

void
cl_tim::happen(class cl_hw *where, enum hw_event he,
	       void *params)
{
  if ((he == EV_CLK_ON) ||
      (he == EV_CLK_OFF))
    {
      cl_clk_event *e= (cl_clk_event *)params;
      if ((e->cath == HW_TIMER) &&
	  (e->id == id))
	clk_enabled= he == EV_CLK_ON;
    }
}

t_mem
cl_tim::read(class cl_memory_cell *cell)
{
  t_mem v= cell->get();
  t_addr a;
  
  if (conf(cell, NULL))
    return v;
  if (!uc->rom->is_owned(cell, &a))
    return v;
  if ((a < base) ||
      (a >= base+32+6))
    return v;

  a-= base;

  if (a == idx.pscrl)
    v= prescaler_preload && 0xff;
  else if (a == idx.pscrh)
    v= (prescaler_preload >> 8) & 0xff;
    
  else if (a == idx.cntrh)
    timer_ls_buffer= regs[idx.cntrl]->get();
  else if (a == idx.cntrl)
    {
      if (bits > 8)
	v= timer_ls_buffer;
    }
  
  return v;
}

void
cl_tim::write(class cl_memory_cell *cell, t_mem *val)
{
  t_addr a;

  if (conf(cell, val))
    return;
  
  if (conf(cell, NULL))
    return;

  *val&= 0xff;
  if (!uc->rom->is_owned(cell, &a))
    return;  
  if ((a < base) ||
      (a >= base+32+6))
    return;
  
  a-= base;
  if (a == idx.cr1)
    {
      u8_t v= cell->get();
      if (!bidir)
	*val&= 0x1f;
      else
	{
	  if ((v & cms))
	    {
	      *val&= ~dir;
	      if (v & dir)
		*val|= dir;
	    }
	}
    }
  else if (a == idx.egr)
    {
      if (*val & ug)
	{
	  update_event();
	  prescaler_cnt= calc_prescaler() - 1;
	  //*val&= ~0x01;
	}
      *val= 0;
    }
  else if (a == idx.pscrh)
    {
      prescaler_ms_buffer= *val;
      //*val= cell->get();
    }
  else if (a == idx.pscrl)
    {
      prescaler_preload= *val;
      if (idx.pscrh > 0)
	prescaler_preload+= prescaler_ms_buffer * 256;
    }
    
  else if (a == idx.arrh)
    {
      if ((regs[idx.cr1]->get() & arpe) != 0)
	{
	  arr_ms_buffer= *val;
	  //*val= cell->get();
	}
    }
  else if (a == idx.arrl)
    {
      u8_t l, h= 0;
      if ((regs[idx.cr1]->get() & arpe) != 0)
	{
	  regs[idx.arrl]->set(l= *val);
	  if (idx.arrh > 0)
	    regs[idx.arrh]->set(h= arr_ms_buffer);
	  if ((regs[idx.cr1]->get() & arpe) == 0)
	    set_counter(h*256 + l);
	}
    }
}

t_mem
cl_tim::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum stm8_tim_cfg)addr)
    {
    case stm8_tim_on:
      if (val)
	{
	  if (*val)
	    on= true;
	  else
	    on= false;
	}
      else
	cell->set(on?1:0);
      break;
    case stm8_tim_nuof_cfg:
      break;
    }
  return cell->get();
}


void
cl_tim::count(void)
{
  u8_t c1= regs[idx.cr1]->get();
  if (get_dir())
    {
      // up
      u16_t arr= get_arr();
      set_counter(cnt+1);
      if (cnt == arr)
	{
	  if ((c1 & cms) == cms0)
	    // edge aligned
	    set_counter(0);
	  else
	    // center aligned
	    regs[idx.cr1]->set(c1|= dir);
	  if ((c1 & udis) == 0)
	    update_event();
	}
    }
  else
    {
      // down
      set_counter(cnt-1);
      if (cnt == 0)
	{
	  if ((c1 & cms) == cms0)
	    // edge aligned
	    set_counter(get_arr());
	  else
	    // center aligned
	    regs[idx.cr1]->set(c1&= ~dir);
	  if ((c1 & udis) == 0)
	    update_event();
	}
    }	      
}

u16_t
cl_tim::get_counter()
{
  if (bits > 8)
    return regs[idx.cntrh]->get()*256 + regs[idx.cntrl]->get();
  return regs[idx.cntrl]->get();
}

u16_t
cl_tim::set_counter(u16_t val)
{
  cnt= val & mask;
  regs[idx.cntrl]->set(val&0xff);
  if (bits > 8)
    regs[idx.cntrh]->set(val>>8);
  return val;
}

void
cl_tim::update_event(void)
{
  u8_t c1= regs[idx.cr1]->get();

  if (c1 & opm)
    regs[idx.cr1]->set_bit0(cen);
  else
    {
      if (get_dir())
	{
	  // up
	  set_counter(0);
	}
      else
	{
	  // down
	  u16_t ar= get_arr();
	  set_counter(ar);
	}
    }
  regs[idx.sr1]->set_bit1(uif);
}

// true: UP, false: down
bool
cl_tim::get_dir()
{
  return !(regs[idx.cr1]->get() & dir);
}

u16_t
cl_tim::get_arr()
{
  u16_t arr= regs[idx.arrl]->get();
  if (bits > 8)
    arr+= regs[idx.arrh]->get() * 256;
  return arr;
}

u16_t
cl_tim::calc_prescaler()
{
  u16_t v;
  switch (pbits)
    {
    case 3:
      v= (1 << (prescaler_preload & 0x07));
      break;
    case 4:
      v= (1 << (prescaler_preload & 0x0f));
      break;
    default: // 16
      v= prescaler_preload + 1;
      break;
    }
  return v;
}

void
cl_tim::print_info(class cl_console_base *con)
{
  u8_t c1= regs[idx.cr1]->get();
  // features
  con->dd_printf("%s %d bit %s counter at 0x%06x\n", get_name(), bits,
		 bidir?"Up/Down":"Up", base);
  // actual values
  con->dd_printf("clk= %s\n", clk_enabled?"enabled":"disabled");
  con->dd_printf("cnt= 0x%04x %d %s\n", cnt, cnt, (c1&cen)?"on":"off");
  con->dd_printf("dir= %s\n", (c1&dir)?"down":"up");
  con->dd_printf("prs= 0x%04x %d of 0x%04x %d\n",
		 prescaler_cnt, prescaler_cnt,
		 calc_prescaler(), calc_prescaler());
  con->dd_printf("arr= 0x%04x %d\n", get_arr(), get_arr());
}


/************************************************************************ 1 */

cl_tim1::cl_tim1(class cl_uc *auc, int aid, t_addr abase):
  cl_tim(auc, aid, abase)
{
}

int
cl_tim1::init(void)
{
  cl_tim::init();
  return 0;
}

cl_tim1_saf::cl_tim1_saf(class cl_uc *auc, int aid, t_addr abase):
  cl_tim1(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  idx.ier	=  4;
  idx.sr1	=  5;
  idx.sr2	=  6;
  idx.egr	=  7;
  idx.ccmr1	=  8;
  idx.ccmr2	=  9;
  idx.ccmr3	= 10;
  idx.ccmr4	= 11;
  idx.ccer1	= 12;
  idx.ccer2	= 13;
  idx.cntrh	= 14;
  idx.cntrl	= 15;
  idx.pscrh	= 16;
  idx.pscrl	= 17;
  idx.arrh	= 18;
  idx.arrl	= 19;
  idx.rcr	= 20;
  idx.ccr1h	= 21;
  idx.ccr1l	= 22;
  idx.ccr2h	= 23;
  idx.ccr2l	= 24;
  idx.ccr3h	= 25;
  idx.ccr3l	= 26;
  idx.ccr4h	= 27;
  idx.ccr4l	= 28;
  idx.bkr	= 29;
  idx.dtr	= 30;
  idx.oisr	= 31;
}

int
cl_tim1_saf::init(void)
{
  class cl_it_src *is;
  cl_tim1::init();
  uc->it_sources->add(is= new cl_it_src(uc, 11,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+11*4, false, false,
					"timer1 update",
					30*10+1));
  is->init();
  return 0;
}

cl_tim1_all::cl_tim1_all(class cl_uc *auc, int aid, t_addr abase):
  cl_tim1(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  idx.der	=  4;
  idx.ier	=  5;
  idx.sr1	=  6;
  idx.sr2	=  7;
  idx.egr	=  8;
  idx.ccmr1	=  9;
  idx.ccmr2	= 10;
  idx.ccmr3	= 11;
  idx.ccmr4	= 12;
  idx.ccer1	= 13;
  idx.ccer2	= 14;
  idx.cntrh	= 15;
  idx.cntrl	= 16;
  idx.pscrh	= 17;
  idx.pscrl	= 18;
  idx.arrh	= 19;
  idx.arrl	= 20;
  idx.rcr	= 21;
  idx.ccr1h	= 22;
  idx.ccr1l	= 23;
  idx.ccr2h	= 24;
  idx.ccr2l	= 25;
  idx.ccr3h	= 26;
  idx.ccr3l	= 27;
  idx.ccr4h	= 28;
  idx.ccr4l	= 29;
  idx.bkr	= 30;
  idx.dtr	= 31;
  idx.oisr	= 32;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim1_all::init(void)
{
  class cl_it_src *is;
  cl_tim1::init();
  uc->it_sources->add(is= new cl_it_src(uc, 23,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+23*4, false, false,
					"timer1 update",
					30*10+1));
  is->init();
  return 0;
}


/********************************************************************** 235 */

cl_tim235::cl_tim235(class cl_uc *auc, int aid, t_addr abase):
  cl_tim(auc, aid, abase)
{
}

int
cl_tim235::init(void)
{
  cl_tim::init();
  return 0;
}


/****** TIM 2 */

cl_tim2_saf_a::cl_tim2_saf_a(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  //idx.cr2	=  1;
  //idx.smcr	=  2;
  //idx.etr	=  3;
  //der=4
  idx.ier	=  1;
  idx.sr1	=  2;
  idx.sr2	=  3;
  idx.egr	=  4;
  idx.ccmr1	=  5;
  idx.ccmr2	=  6;
  idx.ccmr3	=  7;
  //idx.ccmr4	= 10;
  idx.ccer1	=  8;
  idx.ccer2	=  9;
  idx.cntrh	= 10;
  idx.cntrl	= 11;
  //idx.pscrh	= 14;
  idx.pscrl	= 12;
  idx.arrh	= 13;
  idx.arrl	= 14;
  //idx.rcr	= 21;
  idx.ccr1h	= 15;
  idx.ccr1l	= 0x10;
  idx.ccr2h	= 0x11;
  idx.ccr2l	= 0x12;
  idx.ccr3h	= 0x13;
  idx.ccr3l	= 0x14;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 30;
  //idx.dtr	= 31;
  //idx.oisr	= 32;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim2_saf_a::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 4;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 13,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+13*4, false, false,
					"timer2 update",
					30*10+2));
  is->init();
  return 0;
}


cl_tim2_saf_b::cl_tim2_saf_b(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  //idx.cr2	=  1;
  //idx.smcr	=  2;
  //idx.etr	=  3;
  //der=4
  idx.ier	=  3;
  idx.sr1	=  4;
  idx.sr2	=  5;
  idx.egr	=  6;
  idx.ccmr1	=  7;
  idx.ccmr2	=  8;
  idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 10;
  idx.ccer2	= 11;
  idx.cntrh	= 12;
  idx.cntrl	= 13;
  //idx.pscrh	= 14;
  idx.pscrl	= 14;
  idx.arrh	= 15;
  idx.arrl	= 16;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x11;
  idx.ccr1l	= 0x12;
  idx.ccr2h	= 0x13;
  idx.ccr2l	= 0x14;
  idx.ccr3h	= 0x15;
  idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 30;
  //idx.dtr	= 31;
  //idx.oisr	= 32;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim2_saf_b::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 4;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 13,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+13*4, false, false,
					"timer2 update",
					30*10+2));
  is->init();
  return 0;
}


cl_tim2_all::cl_tim2_all(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  idx.der	=  4;
  idx.ier	=  5;
  idx.sr1	=  6;
  idx.sr2	=  7;
  idx.egr	=  8;
  idx.ccmr1	=  9;
  idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  idx.cntrh	= 0x0c;
  idx.cntrl	= 0x0d;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x0e;
  idx.arrh	= 0x0f;
  idx.arrl	= 0x10;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x11;
  idx.ccr1l	= 0x12;
  idx.ccr2h	= 0x13;
  idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  idx.bkr	= 0x15;
  //idx.dtr	= 31;
  idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim2_all::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 3;
  uc->it_sources->add(is= new cl_it_src(uc, 19,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+19*4, false, false,
					"timer2 update",
					30*10+2));
  is->init();
  return 0;
}


cl_tim2_l101::cl_tim2_l101(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  //idx.der	=  4;
  idx.ier	=  4;
  idx.sr1	=  5;
  idx.sr2	=  6;
  idx.egr	=  7;
  idx.ccmr1	=  8;
  idx.ccmr2	= 0x09;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 0x0a;
  //idx.ccer2	= 11;
  idx.cntrh	= 0x0b;
  idx.cntrl	= 0x0c;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x0d;
  idx.arrh	= 0x0e;
  idx.arrl	= 0x0f;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x10;
  idx.ccr1l	= 0x11;
  idx.ccr2h	= 0x12;
  idx.ccr2l	= 0x13;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  idx.bkr	= 0x14;
  //idx.dtr	= 31;
  idx.oisr	= 0x15;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim2_l101::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 3;
  uc->it_sources->add(is= new cl_it_src(uc, 19,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+19*4, false, false,
					"timer2 update",
					30*10+2));
  is->init();
  return 0;
}


/****** TIM 3 */

cl_tim3_saf::cl_tim3_saf(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  //idx.cr2	=  1;
  //idx.smcr	=  2;
  //idx.etr	=  3;
  //der=4
  idx.ier	=  1;
  idx.sr1	=  2;
  idx.sr2	=  3;
  idx.egr	=  4;
  idx.ccmr1	=  5;
  idx.ccmr2	=  6;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	=  7;
  //idx.ccer2	=  8;
  idx.cntrh	=  8;
  idx.cntrl	=  9;
  //idx.pscrh	= 14;
  idx.pscrl	= 0xa;
  idx.arrh	= 0xb;
  idx.arrl	= 0xc;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x0d;
  idx.ccr1l	= 0x0c;
  idx.ccr2h	= 0x0f;
  idx.ccr2l	= 0x10;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 30;
  //idx.dtr	= 31;
  //idx.oisr	= 32;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim3_saf::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 4;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 15,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+15*4, false, false,
					"timer3 update",
					30*10+3));
  is->init();
  return 0;
}


cl_tim3_all::cl_tim3_all(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  idx.der	=  4;
  idx.ier	=  5;
  idx.sr1	=  6;
  idx.sr2	=  7;
  idx.egr	=  8;
  idx.ccmr1	=  9;
  idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  idx.cntrh	= 0x0c;
  idx.cntrl	= 0x0d;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x0e;
  idx.arrh	= 0x0f;
  idx.arrl	= 0x10;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x11;
  idx.ccr1l	= 0x12;
  idx.ccr2h	= 0x13;
  idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  idx.bkr	= 0x15;
  //idx.dtr	= 31;
  idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim3_all::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 3;
  uc->it_sources->add(is= new cl_it_src(uc, 21,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+21*4, false, false,
					"timer3 update",
					30*10+3));
  is->init();
  return 0;
}


cl_tim3_l101::cl_tim3_l101(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  //idx.der	=  4;
  idx.ier	=  4;
  idx.sr1	=  5;
  idx.sr2	=  6;
  idx.egr	=  7;
  idx.ccmr1	=  8;
  idx.ccmr2	= 0x09;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 0x0a;
  //idx.ccer2	= 11;
  idx.cntrh	= 0x0b;
  idx.cntrl	= 0x0c;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x0d;
  idx.arrh	= 0x0e;
  idx.arrl	= 0x0f;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x10;
  idx.ccr1l	= 0x11;
  idx.ccr2h	= 0x12;
  idx.ccr2l	= 0x13;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  idx.bkr	= 0x14;
  //idx.dtr	= 31;
  idx.oisr	= 0x15;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim3_l101::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 3;
  uc->it_sources->add(is= new cl_it_src(uc, 21,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+21*4, false, false,
					"timer3 update",
					30*10+3));
  is->init();
  return 0;
}


/****** TIM 5 */

cl_tim5_saf::cl_tim5_saf(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  //idx.etr	=  3;
  //der=4
  idx.ier	=  3;
  idx.sr1	=  4;
  idx.sr2	=  5;
  idx.egr	=  6;
  idx.ccmr1	=  7;
  idx.ccmr2	=  8;
  idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 0xa;
  idx.ccer2	= 0xb;
  idx.cntrh	= 0xc;
  idx.cntrl	= 0xd;
  //idx.pscrh	= 14;
  idx.pscrl	= 0xe;
  idx.arrh	= 0xf;
  idx.arrl	= 0x10;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x11;
  idx.ccr1l	= 0x12;
  idx.ccr2h	= 0x13;
  idx.ccr2l	= 0x14;
  idx.ccr3h	= 0x15;
  idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 30;
  //idx.dtr	= 31;
  //idx.oisr	= 32;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim5_saf::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 4;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 13,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+13*4, false, false,
					"timer5 update",
					30*10+5));
  is->init();
  return 0;
}


cl_tim5_all::cl_tim5_all(class cl_uc *auc, int aid, t_addr abase):
  cl_tim235(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  idx.etr	=  3;
  idx.der	=  4;
  idx.ier	=  5;
  idx.sr1	=  6;
  idx.sr2	=  7;
  idx.egr	=  8;
  idx.ccmr1	=  9;
  idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  idx.cntrh	= 0x0c;
  idx.cntrl	= 0x0d;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x0e;
  idx.arrh	= 0x0f;
  idx.arrl	= 0x10;
  //idx.rcr	= 21;
  idx.ccr1h	= 0x11;
  idx.ccr1l	= 0x12;
  idx.ccr2h	= 0x13;
  idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  idx.bkr	= 0x15;
  //idx.dtr	= 31;
  idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim5_all::init(void)
{
  class cl_it_src *is;
  cl_tim235::init();
  pbits= 3;
  uc->it_sources->add(is= new cl_it_src(uc, 27,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+27*4, false, false,
					"timer5 update",
					30*10+5));
  is->init();
  return 0;
}


/*********************************************************************** 46 */

cl_tim46::cl_tim46(class cl_uc *auc, int aid, t_addr abase):
  cl_tim(auc, aid, abase)
{
}

int
cl_tim46::init(void)
{
  cl_tim::init();
  pbits= 4;
  return 0;
}


/********* TIM 4 */

cl_tim4_saf_a::cl_tim4_saf_a(class cl_uc *auc, int aid, t_addr abase):
  cl_tim46(auc, aid, abase)
{
  idx.cr1	=  0;
  //idx.cr2	=  1;
  //idx.smcr	=  2;
  //idx.etr	=  3;
  //idx.der	=  4;
  idx.ier	=  1;
  idx.sr1	=  2;
  //idx.sr2	=  7;
  idx.egr	=  3;
  //idx.ccmr1	=  9;
  //idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  //idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  //idx.cntrh	= 0x0c;
  idx.cntrl	= 0x04;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x05;
  //idx.arrh	= 0x0f;
  idx.arrl	= 0x06;
  //idx.rcr	= 21;
  //idx.ccr1h	= 0x11;
  //idx.ccr1l	= 0x12;
  //idx.ccr2h	= 0x13;
  //idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 0x15;
  //idx.dtr	= 31;
  //idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim4_saf_a::init(void)
{
  class cl_it_src *is;
  cl_tim46::init();
  pbits= 3;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 23,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+23*4, false, false,
					"timer4 update",
					30*10+4));
  is->init();
  return 0;
}


cl_tim4_saf_b::cl_tim4_saf_b(class cl_uc *auc, int aid, t_addr abase):
  cl_tim46(auc, aid, abase)
{
  idx.cr1	=  0;
  //idx.cr2	=  1;
  //idx.smcr	=  2;
  //idx.etr	=  3;
  //idx.der	=  4;
  idx.ier	=  3;
  idx.sr1	=  4;
  //idx.sr2	=  7;
  idx.egr	=  5;
  //idx.ccmr1	=  9;
  //idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  //idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  //idx.cntrh	= 0x0c;
  idx.cntrl	= 0x06;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x07;
  //idx.arrh	= 0x0f;
  idx.arrl	= 0x08;
  //idx.rcr	= 21;
  //idx.ccr1h	= 0x11;
  //idx.ccr1l	= 0x12;
  //idx.ccr2h	= 0x13;
  //idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 0x15;
  //idx.dtr	= 31;
  //idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim4_saf_b::init(void)
{
  class cl_it_src *is;
  cl_tim46::init();
  pbits= 3;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 23,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+23*4, false, false,
					"timer4 update",
					30*10+4));
  is->init();
  return 0;
}


cl_tim4_all::cl_tim4_all(class cl_uc *auc, int aid, t_addr abase):
  cl_tim46(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  //idx.etr	=  3;
  idx.der	=  3;
  idx.ier	=  4;
  idx.sr1	=  5;
  //idx.sr2	=  7;
  idx.egr	=  6;
  //idx.ccmr1	=  9;
  //idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  //idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  //idx.cntrh	= 0x0c;
  idx.cntrl	= 0x07;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x08;
  //idx.arrh	= 0x0f;
  idx.arrl	= 0x09;
  //idx.rcr	= 21;
  //idx.ccr1h	= 0x11;
  //idx.ccr1l	= 0x12;
  //idx.ccr2h	= 0x13;
  //idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 0x15;
  //idx.dtr	= 31;
  //idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim4_all::init(void)
{
  class cl_it_src *is;
  cl_tim46::init();
  pbits= 4;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 25,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+25*4, false, false,
					"timer4 update",
					30*10+4));
  is->init();
  return 0;
}


cl_tim4_l101::cl_tim4_l101(class cl_uc *auc, int aid, t_addr abase):
  cl_tim46(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  //idx.etr	=  3;
  //idx.der	=  3;
  idx.ier	=  3;
  idx.sr1	=  4;
  //idx.sr2	=  7;
  idx.egr	=  5;
  //idx.ccmr1	=  9;
  //idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  //idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  //idx.cntrh	= 0x0c;
  idx.cntrl	= 0x06;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x07;
  //idx.arrh	= 0x0f;
  idx.arrl	= 0x08;
  //idx.rcr	= 21;
  //idx.ccr1h	= 0x11;
  //idx.ccr1l	= 0x12;
  //idx.ccr2h	= 0x13;
  //idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 0x15;
  //idx.dtr	= 31;
  //idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim4_l101::init(void)
{
  class cl_it_src *is;
  cl_tim46::init();
  pbits= 4;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 25,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+25*4, false, false,
					"timer4 update",
					30*10+4));
  is->init();
  return 0;
}


/*********** TIM6 */

cl_tim6_saf::cl_tim6_saf(class cl_uc *auc, int aid, t_addr abase):
  cl_tim46(auc, aid, abase)
{
  idx.cr1	=  0;
  idx.cr2	=  1;
  idx.smcr	=  2;
  //idx.etr	=  3;
  //idx.der	=  4;
  idx.ier	=  3;
  idx.sr1	=  4;
  //idx.sr2	=  7;
  idx.egr	=  5;
  //idx.ccmr1	=  9;
  //idx.ccmr2	= 0x0a;
  //idx.ccmr3	=  9;
  //idx.ccmr4	= 10;
  //idx.ccer1	= 0x0b;
  //idx.ccer2	= 11;
  //idx.cntrh	= 0x0c;
  idx.cntrl	= 0x06;
  //idx.pscrh	= 14;
  idx.pscrl	= 0x07;
  //idx.arrh	= 0x0f;
  idx.arrl	= 0x08;
  //idx.rcr	= 21;
  //idx.ccr1h	= 0x11;
  //idx.ccr1l	= 0x12;
  //idx.ccr2h	= 0x13;
  //idx.ccr2l	= 0x14;
  //idx.ccr3h	= 0x15;
  //idx.ccr3l	= 0x16;
  //idx.ccr4h	= 28;
  //idx.ccr4l	= 29;
  //idx.bkr	= 0x15;
  //idx.dtr	= 31;
  //idx.oisr	= 0x16;
  //dcr1=33
  //dcr2=34
  //dmar=35
}

int
cl_tim6_saf::init(void)
{
  class cl_it_src *is;
  cl_tim46::init();
  pbits= 3;
  bidir= false;
  uc->it_sources->add(is= new cl_it_src(uc, 23,
					regs[idx.ier], uie,
					regs[idx.sr1], uif,
					0x8008+23*4, false, false,
					"timer6 update",
					30*10+6));
  is->init();
  return 0;
}


/* End of stm8.src/timer.cc */
