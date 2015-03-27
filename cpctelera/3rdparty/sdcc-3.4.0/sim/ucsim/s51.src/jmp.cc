/*
 * Simulator of microcontrollers (jmp.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

/* Bugs fixed by Sandeep Dutta:
 *	relative<->absolute jump in "jmp @a+dptr"
 */

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>

// local
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"
#include "interruptcl.h"


/*
 * 0x[02468ace]1 2 24 AJMP addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_ajmp_addr(uchar code)
{
  uchar h, l;

  h= (code >> 5) & 0x07;
  l= fetch();
  tick(1);
  PC= (PC & 0xf800) | (h*256 + l);
  return(resGO);
}


/*
 * 0x10 3 12 JBC bit,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jbc_bit_addr(uchar code)
{
  uchar bitaddr, jaddr;

  bitaddr= fetch();
  jaddr  = fetch();
  t_addr a;
  t_mem m;
  class cl_address_space *mem;
  if ((mem= bit2mem(bitaddr, &a, &m)) == 0)
    return(resBITADDR);
  t_mem d= mem->read(a, HW_PORT);
  mem->write(a, d & ~m);
  if (d & m)
    PC= rom->validate_address(PC + (signed char)jaddr);
  tick(1);
  return(resGO);
}


/*
 * 0x02 3 24 LJMP addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_ljmp(uchar code)
{
  PC= fetch()*256 + fetch();
  tick(1);
  return(resGO);
}


/*
 * 0x[13579bdf]1 2 24 ACALL addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_acall_addr(uchar code)
{
  uchar h, l;
  class cl_memory_cell *stck;
  t_mem sp, sp_before/*, sp_after*/;

  h= (code >> 5) & 0x07;
  l= fetch();
  sp_before= sfr->get(SP);
  sp= sfr->wadd(SP, 1);
  //proc_write_sp(sp);
  stck= iram->get_cell(sp);
  stck->write(PC & 0xff); // push low byte
  tick(1);

  sp= /*sp_after*= */sfr->wadd(SP, 1);
  //proc_write_sp(sp);
  stck= iram->get_cell(sp);
  stck->write((PC >> 8) & 0xff); // push high byte
  t_mem pushed= PC;
  PC= (PC & 0xf800) | (h*256 + l);
  class cl_stack_op *so= new cl_stack_call(instPC, PC, pushed, sp_before, sp);
  so->init();
  stack_write(so);
  return(resGO);
}


/*
 * 0x12 3 24 LCALL addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_lcall(uchar code, uint addr, bool intr)
{
  uchar h= 0, l= 0;
  t_mem sp, sp_before/*, sp_after*/;
  class cl_memory_cell *stck;

  if (!addr)
    {
      h= fetch();
      l= fetch();
    }
  sp_before= sfr->get(SP);
  sp= sfr->wadd(SP, 1);
  //proc_write_sp(sp);
  stck= iram->get_cell(sp);
  stck->write(PC & 0xff); // push low byte
  if (!addr)
    tick(1);

  sp= sfr->wadd(SP, 1);
  //proc_write_sp(sp);
  stck= iram->get_cell(sp);
  stck->write((PC >> 8) & 0xff); // push high byte
  t_mem pushed= PC;
  if (addr)
    PC= addr;
  else
    PC= h*256 + l;
  class cl_stack_op *so;
  if (intr)
    so= new cl_stack_intr(instPC, PC, pushed, sp_before, sp/*_after*/);
  else
    so= new cl_stack_call(instPC, PC, pushed, sp_before, sp/*_after*/);
  so->init();
  stack_write(so);
  return(resGO);
}


/*
 * 0x20 3 24 JB bit,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jb_bit_addr(uchar code)
{
  uchar bitaddr, jaddr;
  t_addr a;
  t_mem m;

  class cl_address_space *mem;
  if ((mem= bit2mem(bitaddr= fetch(), &a, &m)) == 0)
    return(resBITADDR);
  tick(1);
  jaddr= fetch();
  if (mem->read(a) & m)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0x22 1 24 RET
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_ret(uchar code)
{
  uchar h= 0, l= 0;
  t_mem sp, sp_before/*, sp_after*/;
  class cl_memory_cell *stck;

  sp= sp_before= sfr->read(SP);
  stck= iram->get_cell(sp);
  h= stck->read();
  sp= sfr->wadd(SP, -1);
  tick(1);

  stck= iram->get_cell(sp);
  l= stck->read();
  sp= sfr->wadd(SP, -1);
  PC= h*256 + l;
  class cl_stack_op *so= new cl_stack_ret(instPC, PC, sp_before, sp/*_after*/);
  so->init();
  stack_read(so);
  return(resGO);
}


/*
 * 0x30 3 24 JNB bit,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jnb_bit_addr(uchar code)
{
  uchar bitaddr, jaddr;
  t_mem m;
  t_addr a;
  class cl_address_space *mem;

  if ((mem= bit2mem(bitaddr= fetch(), &a, &m)) == 0)
    return(resBITADDR);
  tick(1);
  jaddr= fetch();
  if (!(mem->read(a) & m))
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0x32 1 24 RETI
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_reti(uchar code)
{
  uchar h= 0, l= 0;
  t_mem sp, sp_before, sp_after;
  class cl_memory_cell *stck;

  sp= sp_before= sfr->read(SP);
  stck= iram->get_cell(sp);
  h= stck->read();
  sp= sfr->wadd(SP, -1);
  tick(1);

  stck= iram->get_cell(sp);
  l= stck->read();
  sp= sp_after= sfr->wadd(SP, -1);
  PC= h*256 + l;

  interrupt->was_reti= DD_TRUE;
  class it_level *il= (class it_level *)(it_levels->top());
  if (il &&
      il->level >= 0)
    {
      il= (class it_level *)(it_levels->pop());
      delete il;
    }
  class cl_stack_op *so=
    new cl_stack_iret(instPC, PC, sp_before, sp_after);
  so->init();
  stack_read(so);
  return(resGO);
}


/*
 * 0x40 2 24 JC addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jc_addr(uchar code)
{
  uchar jaddr;

  jaddr= fetch();
  tick(1);
  if (SFR_GET_C)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0x50 2 24 JNC addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jnc_addr(uchar code)
{
  uchar jaddr;

  jaddr= fetch();
  tick(1);
  if (!SFR_GET_C)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0x60 2 24 JZ addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jz_addr(uchar code)
{
  uchar jaddr;

  jaddr= fetch();
  tick(1);
  if (!acc->read())
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0x70 2 24 JNZ addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jnz_addr(uchar code)
{
  uchar jaddr;

  jaddr= fetch();
  tick(1);
  if (acc->read())
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0x73 1 24 JMP @A+DPTR
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_jmp_Sa_dptr(uchar code)
{
  PC= rom->validate_address(sfr->read(DPH)*256 + sfr->read(DPL) + acc->read());
  tick(1);
  return(resGO);
}


/*
 * 0x80 2 24 SJMP addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_sjmp(uchar code)
{
  signed char target= fetch();

  PC= rom->validate_address(PC + target);
  tick(1);
  return(resGO);
}


/*
 * 0xb4 3 24 CJNE A,#data,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cjne_a_Sdata_addr(uchar code)
{
  uchar data, jaddr, ac;

  data = fetch();
  jaddr= fetch();
  tick(1);
  SFR_SET_C((ac= acc->read()) < data);
  if (ac != data)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0xb5 3 24 CJNE A,addr,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cjne_a_addr_addr(uchar code)
{
  uchar data, jaddr;
  t_addr a;
  class cl_memory_cell *cell;

  cell= get_direct(a= fetch());
  jaddr= fetch();
  tick(1);
  data= cell->read();
  SFR_SET_C(acc->get() < data);
  if (acc->read() != data)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0xb6-0xb7 3 24 CJNE @Ri,#data,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cjne_Sri_Sdata_addr(uchar code)
{
  uchar data, jaddr;
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  data = fetch();
  jaddr= fetch();
  tick(1);
  t_mem d;
  SFR_SET_C((d= cell->read()) < data);
  if (d != data)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0xb8-0xbf 3 24 CJNE Rn,#data,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cjne_rn_Sdata_addr(uchar code)
{
  uchar data, jaddr;
  class cl_memory_cell *reg;

  reg  = get_reg(code & 0x07);
  data = fetch();
  jaddr= fetch();
  tick(1);
  t_mem r;
  SFR_SET_C((r= reg->read()) < data);
  if (r != data)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0xd5 3 24 DJNZ addr,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_djnz_addr_addr(uchar code)
{
  uchar jaddr;
  class cl_memory_cell *cell;

  cell = get_direct(fetch());
  jaddr= fetch();
  tick(1);
  t_mem d= cell->read(HW_PORT);//cell->wadd(-1);
  d= cell->write(d-1);
  if (d)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/*
 * 0xd8-0xdf 2 24 DJNZ Rn,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_djnz_rn_addr(uchar code)
{
  uchar jaddr;
  class cl_memory_cell *reg;

  reg  = get_reg(code & 0x07);
  jaddr= fetch();
  tick(1);
  t_mem r= reg->wadd(-1);
  if (r)
    PC= rom->validate_address(PC + (signed char)jaddr);
  return(resGO);
}


/* End of s51.src/jmp.cc */
