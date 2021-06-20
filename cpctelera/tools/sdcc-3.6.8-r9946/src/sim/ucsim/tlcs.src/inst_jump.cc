/*
 * Simulator of microcontrollers (tlcs.src/inst_jmp.cc)
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

/* $Id: inst_jump.cc 549 2016-12-13 11:15:58Z drdani $ */

#include "tlcscl.h"


// 1e
int
cl_tlcs::inst_ret()
{
  t_mem pushed_pc;

  exec_ret(PC-1, &pushed_pc);
  PC= pushed_pc;
  return resGO;
}


// 1f
int
cl_tlcs::inst_reti()
{
  t_mem pushed_pc, pushed_af;
  
  exec_pop(PC-1, &pushed_af);
  exec_reti(PC-1, &pushed_pc);
  reg.af= pushed_af;
  PC= pushed_pc;
  return resGO;
}


// CALL
int
cl_tlcs::inst_call(t_addr PC_of_inst, u16_t addr)
{
  exec_call(PC_of_inst, addr, PC);
  PC= addr;
  return resGO;
}


// DJNZ
int
cl_tlcs::inst_djnz_b(i8_t d)
{
  reg.rbc.b--;
  if (reg.rbc.b != 0)
    PC+= d;
  return resGO;
}


// DJNZ BC
int
cl_tlcs::inst_djnz_bc(i8_t d)
{
  reg.bc--;
  if (reg.bc != 0)
    PC+= d;
  return resGO;
}


/* End of tlcs.src/inst_jmp.cc */
