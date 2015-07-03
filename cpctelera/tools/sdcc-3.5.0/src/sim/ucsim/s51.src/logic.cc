/*
 * Simulator of microcontrollers (logic.cc)
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

#include "ddconfig.h"

// prj
#include "stypes.h"

// local
#include "uc51cl.h"
#include "regs51.h"


/*
 * 0x42 2 12 ORL addr,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_addr_a(uchar code)
{
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  cell->write(cell->read(HW_PORT) | acc->read());
  return(resGO);
}


/*
 * 0x43 3 24 ORL addr,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_addr_Sdata(uchar code)
{
  class cl_memory_cell *cell;
  int res= resGO;

  cell= get_direct(fetch());
  t_mem d= fetch();
  cell->write(cell->read(HW_PORT) | d);
  tick(1);
  return(res);
}


/*
 * 0x44 2 12 ORL A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_a_Sdata(uchar code)
{
  uchar d;

  d= acc->read();
  acc->write(d|= fetch());
  return(resGO);
}


/*
 * 0x45 2 12 ORL A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_a_addr(uchar code)
{
  t_mem d;
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  d= acc->read();
  acc->write(d|= cell->read());
  return(resGO);
}


/*
 * 0x46-0x47 1 12 ORL A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_a_Sri(uchar code)
{
  t_mem d;
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  d= acc->read();
  acc->write(d|= cell->read());
  return(resGO);
}


/*
 * 0x48-0x4f 1 12 ORL A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_a_rn(uchar code)
{
  t_mem d;

  d= acc->read();
  acc->write(d|= get_reg(code & 0x07)->read());
  return(resGO);
}


/*
 * 0x52 2 12 ANL addr,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_addr_a(uchar code)
{
  class cl_memory_cell *cell;
  
  cell= get_direct(fetch());
  cell->write(cell->read(HW_PORT) & acc->read());
  return(resGO);
}


/*
 * 0x53 3 24 ANL addr,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_addr_Sdata(uchar code)
{
  class cl_memory_cell *cell;
  t_mem d;

  cell= get_direct(fetch());
  d= fetch();
  cell->write(cell->read(HW_PORT) & d);
  tick(1);
  return(resGO);
}


/*
 * 0x54 2 12 ANL A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_a_Sdata(uchar code)
{
  uchar d;

  d= acc->read();
  acc->write(d & fetch());
  return(resGO);
}


/*
 * 0x55 2 12 ANL A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_a_addr(uchar code)
{
  t_mem d;
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  d= acc->read();
  acc->write(d & cell->read());
  return(resGO);
}


/*
 * 0x56-0x57 1 12 ANL A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_a_Sri(uchar code)
{
  t_mem d;
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  d= acc->read();
  acc->write(d & cell->read());
  return(resGO);
}


/*
 * 0x58-0x5f 1 12 ANL A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_a_rn(uchar code)
{
  uchar d;

  d= acc->read();
  acc->write(d & get_reg(code & 0x07)->read());
  return(resGO);
}


/*
 * 0x62 2 12 XRL addr,A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_xrl_addr_a(uchar code)
{
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  cell->write(cell->read(HW_PORT) ^ acc->read());
  return(resGO);
}


/*
 * 0x63 3 24 XRL addr,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_xrl_addr_Sdata(uchar code)
{
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  cell->write(cell->read(HW_PORT) ^ fetch());
  tick(1);
  return(resGO);
}


/*
 * 0x64 2 12 XRL A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_xrl_a_Sdata(uchar code)
{
  uchar d;

  d= acc->read();
  acc->write(d ^ fetch());
  return(resGO);
}


/*
 * 0x65 2 12 XRL A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_xrl_a_addr(uchar code)
{
  t_mem d;
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  d= acc->read();
  acc->write(d ^ cell->read());
  return(resGO);
}


/*
 * 0x66-0x67 1 12 XRL A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_xrl_a_Sri(uchar code)
{
  t_mem d;
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  d= acc->read();
  acc->write(d ^ cell->read());
  return(resGO);
}


/*
 * 0x68-0x6f 1 12 XRL A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_xrl_a_rn(uchar code)
{
  t_mem d;

  d= acc->read();
  acc->write(d ^ get_reg(code & 0x07)->read());
  return(resGO);
}


/*
 * 0xf4 1 12 CPL A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cpl_a(uchar code)
{
  acc->write(~(acc->read()));
  return(resGO);
}


/* End of s51.src/logic.cc */
