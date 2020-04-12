/*
 * Simulator of microcontrollers (mov.cc)
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
 *	source<->dest bug in "mov direct,direct"
 *	get register in "mov @ri,address"
 */
 
//#include "ddconfig.h"

//#include <stdio.h>

// sim
//#include "memcl.h"

// local
#include "uc51cl.h"
#include "regs51.h"


/*
 * 0x74 2 12 MOV A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_74/*inst_mov_a_Sdata*/(t_mem/*uchar*/ code)
{
  acc->write(fetch());
  //vc.wr++;
  return(resGO);
}


/*
 * 0x75 3 24 MOV addr,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_75/*inst_mov_addr_Sdata*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  cell->write(fetch());
  tick(1);
  vc.wr++;
  return(resGO);
}


/*
 * 0x76-0x77 2 12 MOV @Ri,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_76/*inst_mov_Sri_Sdata*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;
  
  cell= iram->get_cell(R[code & 0x01]->read());
  t_mem d= fetch();
  cell->write(d);
  //vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0x78-0x7f 2 12 MOV Rn,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_78/*inst_mov_rn_Sdata*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *reg;

  reg= R[code & 0x07];
  reg->write(fetch());
  //vc.wr++;
  return(resGO);
}


/*
 * 0x83 1 24 MOVC A,@A+PC
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_83/*inst_movc_a_Sa_pc*/(t_mem/*uchar*/ code)
{
  acc->write(rom->read(PC + acc->read()));
  tick(1);
  vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x85 3 24 MOV addr,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_85/*inst_mov_addr_addr*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *d, *s;

  /* SD reversed s & d here */
  s= get_direct(fetch());
  d= get_direct(fetch());
  d->write(s->read());
  tick(1);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0x86-0x87 2 24 MOV addr,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_86/*inst_mov_addr_Sri*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *d, *s;

  d= get_direct(fetch());
  s= iram->get_cell(R[code & 0x01]->read());
  d->write(s->read());
  tick(1);
  vc.rd++;//= 2;
  vc.wr++;
  return(resGO);
}


/*
 * 0x88-0x8f 2 24 MOV addr,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_88/*inst_mov_addr_rn*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  cell->write(R[code & 0x07]->read());
  tick(1);
  //vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0x90 3 24 MOV DPTR,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_90/*inst_mov_dptr_Sdata*/(t_mem/*uchar*/ code)
{
  /*sfr*/dptr->write(/*DPH*/1, fetch());
  /*sfr*/dptr->write(/*DPL*/0, fetch());
  tick(1);
  vc.wr+= 2;
  return(resGO);
}


/*
 * 0x93 1 24 MOVC A,@A+DPTR
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_93/*inst_movc_a_Sa_dptr*/(t_mem/*uchar*/ code)
{
  u16_t h= /*sfr*/dptr->read(/*DPH*/1);
  u16_t l= /*sfr*/dptr->read(/*DPL*/0);
  acc->write(rom->read(h*256 + l +  acc->read()));
  tick(1);
  vc.rd+= 3;//4;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xa6-0xa7 2 24 MOV @Ri,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_a6/*inst_mov_Sri_addr*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *d, *s;

  d= iram->get_cell(R[code & 0x01]->read());
  s= get_direct(fetch());
  d->write(s->read());
  tick(1);
  vc.rd++;//= 2;
  vc.wr++;
  return(resGO);
}


/*
 * 0xa8-0xaf 2 24 MOV Rn,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_a8/*inst_mov_rn_addr*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *reg, *cell;

  reg = R[code & 0x07];
  cell= get_direct(fetch());
  reg->write(cell->read());
  tick(1);
  vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xc0 2 24 PUSH addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_c0/*inst_push*/(t_mem/*uchar*/ code)
{
  t_addr sp, sp_before/*, sp_after*/;
  t_mem data;
  class cl_memory_cell *stck, *cell;

  cell= get_direct(fetch());
  sp_before= sfr->get(SP);
  sp= /*sp_after= */sfr->wadd(SP, 1);
  stck= iram->get_cell(sp);
  stck->write(data= cell->read());
  class cl_stack_op *so=
    new cl_stack_push(instPC, data, sp_before, sp/*_after*/);
  so->init();
  stack_write(so);
  tick(1);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xc5 2 12 XCH A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_c5/*inst_xch_a_addr*/(t_mem/*uchar*/ code)
{
  t_mem temp;
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  temp= acc->read();
  acc->write(cell->read());
  cell->write(temp);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xc6-0xc7 1 12 XCH A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_c6/*inst_xch_a_Sri*/(t_mem/*uchar*/ code)
{
  t_mem temp;
  class cl_memory_cell *cell;

  cell= iram->get_cell(R[code & 0x01]->read());
  temp= acc->read();
  acc->write(cell->read());
  cell->write(temp);
  vc.rd++;//= 3;
  vc.wr++;//= 2;
  return(resGO);
}


/*
 * 0xc8-0xcf 1 12 XCH A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_c8/*inst_xch_a_rn*/(t_mem/*uchar*/ code)
{
  t_mem temp;
  class cl_memory_cell *reg;

  reg = R[code & 0x07];
  temp= acc->read();
  acc->write(reg->read());
  reg->write(temp);
  //vc.rd+= 2;
  //vc.wr+= 2;
  return(resGO);
}


/*
 * 0xd0 2 24 POP addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_d0/*inst_pop*/(t_mem/*uchar*/ code)
{
  t_addr sp, sp_before/*, sp_after*/;
  t_mem data;
  class cl_memory_cell *cell, *stck;

  sp_before= sfr->get(SP);
  cell= get_direct(fetch());
  stck= iram->get_cell(/*sfr->get(SP)*/sp_before);
  /* Order of decrement and write changed to fix POP SP, reported by
     Alexis Pavlov <alexis.pavlov@certess.com> */
  sp= sfr->wadd(SP, -1);
  cell->write(data= stck->read());
  tick(1);
  class cl_stack_op *so=
    new cl_stack_pop(instPC, data, sp_before, sp/*_after*/);
  so->init();
  stack_read(so);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xd6-0xd7 1 12 XCHD A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_d6/*inst_xchd_a_Sri*/(t_mem/*uchar*/ code)
{
  t_mem temp, d;
  class cl_memory_cell *cell;

  cell= iram->get_cell(R[code & 0x01]->read());
  temp= (d= cell->read()) & 0x0f;
  cell->write((d & 0xf0) | (acc->read() & 0x0f));
  acc->write((acc->get() & 0xf0) | temp);
  vc.rd++;//= 3;
  vc.wr++;//= 2;
  return(resGO);
}


/*
 * 0xe0 1 24 MOVX A,@DPTR
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_e0/*inst_movx_a_Sdptr*/(t_mem/*uchar*/ code)
{
  u16_t h= /*sfr*/dptr->read(/*DPH*/1);
  u16_t l= /*sfr*/dptr->read(/*DPL*/0);
  acc->write(xram->read(h*256 + l));
  tick(1);
  vc.rd+= 3;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xe2-0xe3 1 24 MOVX A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_e2/*inst_movx_a_Sri*/(t_mem/*uchar*/ code)
{
  t_mem d;

  d= R[code & 0x01]->read();
  acc->write(xram->read(sfr->read(P2)*256 + d));
  tick(1);
  vc.rd++;//= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xe5 2 12 MOV A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_e5/*inst_mov_a_addr*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;
  t_addr address= fetch();
  
  /* If this is ACC, it is an invalid instruction */
  if (address == ACC)
    {
      //sim->app->get_commander()->
      //debug("Invalid Instruction : E5 E0  MOV A,ACC  at  %06x\n", PC);
      inst_unknown();
    }
  else
    {
      cell= get_direct(address);
      acc->write(cell->read());
      vc.rd++;
      //vc.wr++;
    }
  
  return(resGO);
}


/*
 * 0xe6-0xe7 1 12 MOV A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_e6/*inst_mov_a_Sri*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;

  cell= iram->get_cell(R[code & 0x01]->read());
  acc->write(cell->read());
  vc.rd++;//= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xe8-0xef 1 12 MOV A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_e8/*inst_mov_a_rn*/(t_mem/*uchar*/ code)
{
  acc->write(R[code & 0x07]->read());
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xf0 1 24 MOVX @DPTR,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_f0/*inst_movx_Sdptr_a*/(t_mem/*uchar*/ code)
{
  u16_t h= /*sfr*/dptr->read(/*DPH*/1);
  u16_t l= /*sfr*/dptr->read(/*DPL*/0);  
  xram->write(h*256 + l, acc->read());
  tick(1);
  vc.rd+= 2;//3;
  vc.wr++;
  return(resGO);
}


/*
 * 0xf2-0xf3 1 24 MOVX @Ri,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_f2/*inst_movx_Sri_a*/(t_mem/*uchar*/ code)
{
  t_mem d, v;
  t_addr a;
  
  d= R[code & 0x01]->read();
  a= sfr->read(P2)*256 + d;
  v= acc->read();
  xram->write(a, v);
  tick(1);
  vc.rd++;//= 2;
  vc.wr++;
  return(resGO);
}


/*
 * 0xf5 2 12 MOV addr,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_f5/*inst_mov_addr_a*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;
  
  cell= get_direct(fetch());
  cell->write(acc->read());
  //vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xf6-0xf7 1 12 MOV @Ri,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_f6/*inst_mov_Sri_a*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *cell;

  cell= iram->get_cell(R[code & 0x01]->read());
  cell->write(acc->read());
  //vc.rd+= 2;
  vc.wr++;
  return(resGO);
}


/*
 * 0xf8-0xff 1 12 MOV Rn,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_f8/*inst_mov_rn_a*/(t_mem/*uchar*/ code)
{
  class cl_memory_cell *reg;

  reg= R[code & 0x07];
  reg->write(acc->read());
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/* End of s51.src/mov.cc */
