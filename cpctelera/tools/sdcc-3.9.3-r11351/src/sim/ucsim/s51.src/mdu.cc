/*
 * Simulator of microcontrollers (s51.src/mdu.cc)
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

#include "uc51cl.h"

#include "mducl.h"

cl_mdu::cl_mdu(class cl_uc *auc, int aid):
  cl_hw(auc, HW_CALC, aid, "mdu")
{
}

void
cl_mdu::op_32udiv16(void)
{
  u32_t dend= v[3]*256*256*256 + v[2]*256*256 + v[1]*256 + v[0];
  u16_t dor= v[5]*256 + v[4];
  u32_t quo= 0;
  u16_t rem= 0;
  if (dor == 0)
    set_ovr(true);
  else
    {
      quo= dend / dor;
      rem= dend % dor;
      set_ovr(false);
      //printf("\nSIM %u/%u=%u,%u %x,%x\n", dend, dor, quo, rem, quo, rem);
    }
  regs[0]->set(quo & 0xff);
  regs[1]->set((quo>>8) & 0xff);
  regs[2]->set((quo>>16) & 0xff);
  regs[3]->set((quo>>24) & 0xff);
  regs[4]->set(rem & 0xff);
  regs[5]->set((rem>>8) & 0xff);
  /*{
    int j;
    for (j=0;j<6;j++)
    {
    printf("  REG[%d]=%02x/%02x %p\n",j,regs[j]->get(),sfr->get(0xe9+j), regs[j]);
    }
    }*/
}

void
cl_mdu::op_16udiv16(void)
{
  // 16/16
  u16_t dend= v[1]*256 + v[0];
  u16_t dor= v[5]*256 + v[4];
  u16_t quo= 0;
  u16_t rem= 0;
  if (dor == 0)
    set_ovr(true);
  else
    {
      quo= dend / dor;
      rem= dend % dor;
      set_ovr(false);
    }
  regs[0]->set(quo & 0xff);
  regs[1]->set((quo>>8) & 0xff);
  regs[4]->set(rem & 0xff);
  regs[5]->set((rem>>8) & 0xff);
}

void
cl_mdu::op_16umul16(void)
{
  u16_t mand= v[1]*256 + v[0];
  u16_t mor= v[5]*256 + v[4];
  u32_t pr= mand * mor;
  regs[0]->set(pr & 0xff);
  regs[1]->set((pr>>8) & 0xff);
  regs[2]->set((pr>>16) & 0xff);
  regs[3]->set((pr>>24) & 0xff);
  if (pr > 0xffff)
    set_ovr(true);
  else
    set_ovr(false);
  regs[4]->set(v[4]); // behavior of xc88x
  regs[5]->set(v[5]);
}

void
cl_mdu::op_norm(void)
{
  u32_t d;
  
  d= v[3]*256*256*256 + v[2]*256*256 + v[1]*256 + v[0];
  if (d == 0)
    set_steps(0);
  else if (d & 0x80000000)
    set_ovr(true);
  else
    {
      int i;
      for (i= 0; (d&0x80000000)==0; i++)
	d<<= 1;
      set_steps(i);
      //printf("NORM d=%x i=%d\n", d, i);
    }
  regs[0]->set(d & 0xff);
  regs[1]->set((d>>8) & 0xff);
  regs[2]->set((d>>16) & 0xff);
  regs[3]->set((d>>24) & 0xff);
}

/* Logical shift */

void
cl_mdu::op_lshift(void)
{
  u32_t d;
  
  d= v[3]*256*256*256 + v[2]*256*256 + v[1]*256 + v[0];
  if (dir_right())
    d<<= get_steps();
  else
    d>>= get_steps();
  regs[0]->set(d & 0xff);
  regs[1]->set((d>>8) & 0xff);
  regs[2]->set((d>>16) & 0xff);
  regs[3]->set((d>>24) & 0xff);
}


/*                                                                     517
 */

cl_mdu517::cl_mdu517(class cl_uc *auc, int aid):
  cl_mdu(auc, aid)
{
}

int
cl_mdu517::init(void)
{
  int i;
  class cl_51core *u= (cl_51core*)uc;

  cl_hw::init();
  
  con= register_cell(u->sfr, 0xef);
  for (i= 0; i<6; i++)
    {
      regs[i]= register_cell(u->sfr, 0xe9+i);
      v[i]= regs[i]->get();
    }
  nuof_writes= 0;
  writes= 0xffffffffffffULL;
  //calcing= 0;
  return 0;
}

t_mem
cl_mdu517::read(class cl_memory_cell *cell)
{
  cl_address_space *sfr= ((cl_51core*)uc)->sfr;
  t_addr a;
  t_mem v= cell->get();
  
  if (conf(cell, NULL))
    return v;
  if (cell == con)
    cell->set(v&= ~0x80);
  else if (sfr->is_owned(cell, &a))
    {
      a-= 0xe9;
      if ((a < 0) ||
	  (a > 5))
	return v;
    }
  return v;
}

void
cl_mdu517::write(class cl_memory_cell *cell, t_mem *val)
{
  cl_address_space *sfr= ((cl_51core*)uc)->sfr;
  t_addr a;
  u8_t ar= con->get() & ~0x80;
  
  if (conf(cell, val))
    return;

  if (sfr->is_owned(cell, &a))
    {
      // if (a==0xee) printf(" WRITE EE %02x\n", *val);
      a-= 0xe9;
      if ((a < 0) ||
	  (a > 6))
	{
	  return;
	}
      /*if (calcing)
	{
	  regs[6]->set(ar | 0x80);
	  return;
	  }*/
      if (a == 0)
	{
	  writes= 0xffffffffffffULL;
	  nuof_writes= 0;
	  set_err(false);
	}
      if (nuof_writes > 5)
	{
	  set_err(true);
	  return;
	}
      writes&= ~(0xffL << (nuof_writes*8));
      writes|= ((u64_t)a << (nuof_writes*8));
      if (a == 6)
	{
	  writes= 0xff0603020100ULL; // force norm/shift
	  con->set(ar= *val & 0x7f);
	  set_err(false);
	}
      else
	{
	  v[a]= *val;
	  nuof_writes++;
	}
      
      switch (writes)
	{
	  //   665544332211
	case 0x050403020100ULL:
	  {
	    // 32/16
	    op_32udiv16();
	    //calcing= 6;
	    writes= 0xffffffffffffULL;
	    nuof_writes= 0;
	    break;
	  }
	  //   665544332211
	case 0xffff05040100ULL:
	  {
	    op_16udiv16();
	    //calcing= 6;
	    writes= 0xffffffffffffULL;
	    nuof_writes= 0;
	    break;
	  }
	  //   665544332211
	case 0xffff05010400ULL:
	  {
	    // 16*16
	    op_16umul16();
	    writes= 0xffffffffffffULL;
	    nuof_writes= 0;
	    break;
	  }
	  //   665544332211
	case 0xff0603020100ULL:
	  {
	    // norm, shift
	    if ((ar & 0x1f) == 0)
	      op_norm();
	    else
	      op_lshift();
	    writes= 0xffffffffffffULL;
	    nuof_writes= 0;
	    break;
	  }
	default:
	  if (nuof_writes > 5)
	    {
	      set_err(true);
	      writes= 0xffffffffffffULL;
	      nuof_writes= 0;
	    }
	  break;
	}
      if (a < 6)
	*val= regs[a]->get();
      else if (cell == con)
	*val= con->get();
    }
}

bool
cl_mdu517::dir_right(void)
{
  return (con->get() & 0x20) != 0;
}

void
cl_mdu517::set_steps(int steps)
{
  t_mem val= con->get();
  val&= ~0x1f;
  steps&= 0x1f;
  con->set(val | steps);
}

int
cl_mdu517::get_steps(void)
{
  return con->get() & 0x1f;
}

void
cl_mdu517::set_ovr(bool val)
{
  if (val)
    con->set_bit1(0x40);
  else
    con->set_bit0(0x40);
}

void
cl_mdu517::set_err(bool val)
{
  if (val)
    con->set_bit1(0x80);
  else
    con->set_bit0(0x80);
}

t_mem
cl_mdu517::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  return cell->get();
}

/*                                                            XC88X
 */

cl_mdu88x::cl_mdu88x(class cl_uc *auc, int aid):
  cl_mdu(auc, aid)
{
}

int
cl_mdu88x::init(void)
{
  int i;
  class cl_51core *u= (cl_51core*)uc;

  cl_hw::init();
  
  stat= register_cell(u->sfr, 0xb0);
  con= register_cell(u->sfr, 0xb1);
  for (i= 0; i<6; i++)
    {
      regs[i]= register_cell(u->sfr, 0xb2+i);
      v[i]= regs[i]->get();
    }
  //calcing= 0;
  return 0;
}

t_mem
cl_mdu88x::read(class cl_memory_cell *cell)
{
  cl_address_space *sfr= ((cl_51core*)uc)->sfr;
  t_addr a;
  t_mem val= cell->get();
  
  if (conf(cell, NULL))
    return val;
  if (cell == stat)
    {}
  else if (cell == con)
    {}
  else if (sfr->is_owned(cell, &a))
    {
      a-= 0xb2;
      if ((a < 0) ||
	  (a > 5))
	{
	  if (con->get() & 0x20)
	    val= regs[a]->get();
	  else
	    val= v[a];
	}
    }
  return val;
}

void
cl_mdu88x::write(class cl_memory_cell *cell, t_mem *val)
{
  cl_address_space *sfr= ((cl_51core*)uc)->sfr;
  t_addr a;
  
  if (conf(cell, val))
    return;

  if (cell == stat)
    {}
  else if (cell == con)
    {
      if (((con->get() & 0x10) == 0) &&
	  (*val & 0x10))
	{
	  // START
	  if (busy())
	    // skip when already BUSY
	    return;
	  con->set(*val&= ~0x10);
	  set_bsy(true);
	  switch (*val & 0x0f)
	    {
	    case 0:
	      op_16umul16();
	      ticks= 16 / uc->clock_per_cycle();
	      break;
	    case 1:
	      op_16udiv16();
	      ticks= 16 / uc->clock_per_cycle();
	      break;
	    case 2:
	      op_32udiv16();
	      ticks= 32 / uc->clock_per_cycle();
	      break;
	    case 3:
	      ticks= (get_steps()+1) / uc->clock_per_cycle();
	      op_lshift();
	      break;
	    case 4:
	      op_16smul16();
	      ticks= 16 / uc->clock_per_cycle();
	      break;
	    case 5:
	      op_16sdiv16();
	      ticks= 16 / uc->clock_per_cycle();
	      break;
	    case 6:
	      op_32sdiv16();
	      ticks= 32 / uc->clock_per_cycle();
	      break;
	    case 7:
	      ticks= (get_steps()+1) / uc->clock_per_cycle();
	      op_ashift();
	      break;
	    case 8:
	      op_norm();
	      ticks= (get_steps()+1) / uc->clock_per_cycle();
	      break;
	    default:
	      {
		// ERROR, unknown opcode
		set_bsy(false);
		set_err(true);
	      }
	    }
	}
    }
  else if (sfr->is_owned(cell, &a))
    {
      a-= 0xb2;
      if ((a < 0) ||
	  (a > 5))
	return;
      /*if (calcing)
	{
	  regs[6]->set(ar | 0x80);
	  return;
	  }*/
      v[a]= *val;
      if (cell == stat)
	*val= stat->get();
      else if (cell == con)
	*val= con->get();
      else if (a < 6)
	*val= regs[a]->get();
    }
}

int
cl_mdu88x::tick(int cycles)
{
  if (busy())
    {
      ticks-= cycles;
      if (ticks < 0)
	ticks= 0;
      set_bsy(false);
    }
  return 0;
}


void
cl_mdu88x::op_32sdiv16(void)
{
  i32_t dend= v[3]*256*256*256 + v[2]*256*256 + v[1]*256 + v[0];
  i16_t dor= v[5]*256 + v[4];
  i32_t quo= 0;
  i16_t rem= 0;
  if (dor == 0)
    set_ovr(true);
  else
    {
      quo= dend / dor;
      rem= dend % dor;
      set_ovr(false);
      //printf("\nSIM %d/%d=%d,%d %x,%x\n", dend, dor, quo, rem, quo, rem);
    }
  regs[0]->set(quo & 0xff);
  regs[1]->set((quo>>8) & 0xff);
  regs[2]->set((quo>>16) & 0xff);
  regs[3]->set((quo>>24) & 0xff);
  regs[4]->set(rem & 0xff);
  regs[5]->set((rem>>8) & 0xff);
  /*{
    int j;
    for (j=0;j<6;j++)
    {
    printf("  REG[%d]=%02x/%02x %p\n",j,regs[j]->get(),sfr->get(0xe9+j), regs[j]);
    }
    }*/
}

void
cl_mdu88x::op_16sdiv16(void)
{
  // 16/16
  i16_t dend= v[1]*256 + v[0];
  i16_t dor= v[5]*256 + v[4];
  i16_t quo= 0;
  i16_t rem= 0;
  if (dor == 0)
    set_ovr(true);
  else
    {
      quo= dend / dor;
      rem= dend % dor;
      set_ovr(false);
    }
  regs[0]->set(quo & 0xff);
  regs[1]->set((quo>>8) & 0xff);
  regs[4]->set(rem & 0xff);
  regs[5]->set((rem>>8) & 0xff);
}

void
cl_mdu88x::op_16smul16(void)
{
  i16_t mand= v[1]*256 + v[0];
  i16_t mor= v[5]*256 + v[4];
  i32_t pr= mand * mor;
  regs[0]->set(pr & 0xff);
  regs[1]->set((pr>>8) & 0xff);
  regs[2]->set((pr>>16) & 0xff);
  regs[3]->set((pr>>24) & 0xff);
  if (pr > 0xffff)
    set_ovr(true);
  else
    set_ovr(false);
  regs[4]->set(v[4]); // behavior of xc88x
  regs[5]->set(v[5]);
}

/* Arithmetic shift */

void
cl_mdu88x::op_ashift(void)
{
  i32_t d;
  
  d= v[3]*256*256*256 + v[2]*256*256 + v[1]*256 + v[0];
  if (dir_right())
    d<<= get_steps();
  else
    d>>= get_steps();
  regs[0]->set(d & 0xff);
  regs[1]->set((d>>8) & 0xff);
  regs[2]->set((d>>16) & 0xff);
  regs[3]->set((d>>24) & 0xff);
}


bool
cl_mdu88x::dir_right(void)
{
  return (v[4] & 0x20) != 0;
}

void
cl_mdu88x::set_steps(int steps)
{
  regs[4]->set(steps & 0x1f);
}

int
cl_mdu88x::get_steps(void)
{
  return v[4] & 0x1f;
}

void
cl_mdu88x::set_ovr(bool val)
{
}

void
cl_mdu88x::set_err(bool val)
{
  if (val)
    stat->set_bit1(0x02);
  else
    stat->set_bit0(0x02);
}

void
cl_mdu88x::set_bsy(bool val)
{
  if (val)
    stat->set_bit1(0x04);
  else
    stat->set_bit0(0x04);
}

bool
cl_mdu88x::busy(void)
{
  return (stat->get() & 0x04) != 0;
}


t_mem
cl_mdu88x::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  return cell->get();
}


/* End of s51.src/mdu.cc */
