/*
 * Simulator of microcontrollers (timer.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include "i8020cl.h"

#include "timercl.h"


cl_timer::cl_timer(class cl_uc *auc):
  cl_hw(auc, HW_TIMER, 0, "timer")
{
  pre16= 0;
}

int
cl_timer::init(void)
{
  cl_hw::init();
  //uc->vars->add("timer_on", cfg, tcfg_on, cfg_help(tcfg_on));
  uc->vars->add("timer0_mode", cfg, tcfg_mode, cfg_help(tcfg_mode));
  uc->vars->add("timer0_pre16", cfg, tcfg_pre16, cfg_help(tcfg_pre16));
  uc->vars->add("timer0_pre", cfg, tcfg_pre, cfg_help(tcfg_pre));
  uc->vars->add("timer", cfg, tcfg_tmr, cfg_help(tcfg_tmr));
  uc->vars->add("timer0", cfg, tcfg_tmr, cfg_help(tcfg_tmr));
  uc->vars->add("timer0_overflow", cfg, tcfg_ovflag, cfg_help(tcfg_ovflag));
  uc->vars->add("timer0_flag", cfg, tcfg_tflag, cfg_help(tcfg_tflag));
  uc->vars->add("timer0_int_enabled", cfg, tcfg_ien, cfg_help(tcfg_ien));
  return 0;
}

void
cl_timer::reset(void)
{
  pre= pre16= tmr= 0;
  mode= tm_stop;
  int_enabled= 0;
  overflow_flag= 0;
  timer_flag= 0;
}

void
cl_timer::print_info(class cl_console_base *con)
{
  con->dd_printf("pre16=%u pre32=%u tmr=%u,0x%02x ",
		 pre16, pre, tmr, tmr);
  con->dd_printf("mode=%s ",
		 (mode==tm_counter)?"COUNTER":
		 (mode==tm_timer)?"TIMER":
		 "STOP");
  con->dd_printf("tflag=%s ovf=%s int %s",
		 timer_flag?"ON":"off",
		 overflow_flag?"ON":"off",
		 int_enabled?"enabled":"disabled");
  con->dd_printf("\n");
}

int
cl_timer::tick(int cycles)
{
  unsigned int c;
  if (!on)
    return resGO;
  pre16+= cycles;
  c= pre16/16;
  if (c)
    {
      pre16%= 16;
      pre+= c;
      c= pre/32;
      if (c)
	pre%= 32;
    }
  switch (mode)
    {
    case tm_stop:
      break;
    case tm_counter:
      break;
    case tm_timer:
      do_timer(c);
      break;
    }
  return resGO;
}

void
cl_timer::do_timer(unsigned int cyc)
{
  tmr+= cyc;
  if (tmr > 0xff)
    {
      do_overflow();
    }
  tmr&= 0xff;
  cfg_write(tcfg_tmr, tmr);
}

void
cl_timer::do_counter(unsigned int cyc)
{
  if (mode == tm_counter)
    {
      tmr+= cyc;
      if (tmr > 0xff)
	{
	  do_overflow();
	}
      tmr&= 0xff;
      cfg_write(tcfg_tmr, tmr);
    }
}

void
cl_timer::do_overflow(void)
{
  cfg_write(tcfg_tflag, 1);
  if (int_enabled)
    cfg_write(tcfg_ovflag, 1);
}

const char *
cl_timer::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case tcfg_on: return cl_hw::cfg_help(addr);
    case tcfg_mode: return "Mode of timer (0:stop, 1:counter, 2:timer, RW)";
    case tcfg_pre16: return "Pre divider of input clock, divs by 16 (int, RW)";
    case tcfg_pre: return "Prescaler of timer, divs by 32 (int, RW)";
    case tcfg_tmr: return "Value of timer (8 bit, RW)";
    case tcfg_ovflag: return "Overflow flag (bool, RW)";
    case tcfg_tflag: return "Timer flag (bool, RW)";
    case tcfg_ien: return "Enable of interrupt request (bool, RW)";
    }
  return "Not used";
}

t_mem
cl_timer::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch (addr)
    {
    case tcfg_on: return cl_hw::conf_op(cell, addr, val);
    case tcfg_mode:
      if (val)
	{
	  mode= (enum timer_modes)(*val % 3);
	  if (mode != tm_stop)
	    pre= 0;
	}
      cell->set(mode);
      break;
    case tcfg_pre16:
      if (val) pre16= *val % 16;
      cell->set(pre16);
      break;
    case tcfg_pre:
      if (val) pre= *val % 32;
      cell->set(pre);
      break;
    case tcfg_tmr:
      if (val)
	tmr= *val & 0xff;
      cell->set(tmr);
      break;
    case tcfg_ien:
      if (val)
	{
	  if (!(int_enabled= (*val)?1:0))
	    overflow_flag= 0;
	}
      cell->set(int_enabled?1:0);
      break;
    case tcfg_tflag:
      if (val)
	timer_flag= (*val)?1:0;
      cell->set(timer_flag?1:0);
      break;
    case tcfg_ovflag:
      if (val)
	overflow_flag= (*val)?1:0;
      cell->set(overflow_flag?1:0);
      break;
    }
  return cell->get();
}


int
CL2::ENTCNTI(MP)
{
  if (timer)
    timer->int_enabled= 1;
  return resGO;
}

int
CL2::DISTCNTI(MP)
{
  if (timer)
    {
      timer->int_enabled= 0;
      timer->overflow_flag= 0;
    }
  return resGO;
}

int
CL2::JTF(MP)
{
  jif(timer->timer_flag);
  timer->timer_flag= 0;
  return resGO;
}

int
CL2::MOVAT(MP)
{
  cA.W(timer->tmr);
  return resGO;
}

int
CL2::MOVTA(MP)
{
  timer->cfg_write(tcfg_tmr, rA);
  return resGO;
}

int
CL2::STRTCNT(MP)
{
  timer->cfg_write(tcfg_mode, tm_counter);
  return resGO;
}

int
CL2::STRTT(MP)
{
  timer->cfg_write(tcfg_mode, tm_timer);
  return resGO;
}

int
CL2::STOPTCNT(MP)
{
  timer->cfg_write(tcfg_mode, tm_stop);
  return resGO;
}

/* End of i8048.src/timer.cc */
